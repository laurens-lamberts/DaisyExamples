#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "daisysp.h"
// #include "modules/wavplayer/wavplayer.h"

// Wav files need to be 16 bit - 1 channel (though 2 channels seems fine?) - 48kHz

using namespace daisy;
using namespace daisysp;

DaisySeed      hardware;
SdmmcHandler   sdcard;
FatFSInterface fsi;
WavPlayer      sampler1Left;
WavPlayer      sampler1Right;
WavPlayer      sampler2Left;
WavPlayer      sampler2Right;
WavPlayer      sampler3Left;
WavPlayer      sampler3Right;
WavPlayer      sampler4Left;
WavPlayer      sampler4Right;
WavPlayer      sampler5Left;
WavPlayer      sampler5Right;
WavPlayer      sampler6Left;
WavPlayer      sampler6Right;
WavPlayer      sampler7Left;
WavPlayer      sampler7Right;
WavPlayer      sampler8Left;
WavPlayer      sampler8Right;

Led           led1;
AnalogControl pot1;

// define buffers
// these are initialized globally on the SDRAM memory sector
#define BUFSIZE 65536
int16_t DSY_SDRAM_BSS sample1BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample1BufferRight[BUFSIZE];
int16_t DSY_SDRAM_BSS sample2BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample2BufferRight[BUFSIZE];
int16_t DSY_SDRAM_BSS sample3BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample3BufferRight[BUFSIZE];
int16_t DSY_SDRAM_BSS sample4BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample4BufferRight[BUFSIZE];
int16_t DSY_SDRAM_BSS sample5BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample5BufferRight[BUFSIZE];
int16_t DSY_SDRAM_BSS sample6BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample6BufferRight[BUFSIZE];
int16_t DSY_SDRAM_BSS sample7BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample7BufferRight[BUFSIZE];
int16_t DSY_SDRAM_BSS sample8BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample8BufferRight[BUFSIZE];

constexpr int NUMBER_OF_ADC_CHANNELS = 2;

float intToFloat(uint16_t in)
{
    return static_cast<float>(in) / 65536.0f;
}

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float samp_out_left  = 0.0f;
    float samp_out_right = 0.0f;

    float mixKnobLevel = hardware.adc.GetFloat(0);
    float volumeKnob1Level
        = intToFloat(hardware.adc.GetMux(1, 0)); //hardware.adc.GetFloat(1);
    float volumeKnob2Level
        = intToFloat(hardware.adc.GetMux(1, 1)); //hardware.adc.GetFloat(2);
    float volumeKnob3Level
        = intToFloat(hardware.adc.GetMux(1, 2)); //hardware.adc.GetFloat(3);
    float volumeKnob4Level = 0.0f;
    float volumeKnob5Level = 0.0f;
    float volumeKnob6Level = 0.0f;
    float volumeKnob7Level = 0.0f;
    float volumeKnob8Level = 0.0f;

    for(size_t i = 0; i < size; i += 2)
    {
        samp_out_left += s162f(sampler1Left.Stream()) * volumeKnob1Level;
        samp_out_right += s162f(sampler1Right.Stream()) * volumeKnob1Level;

        samp_out_left += s162f(sampler2Left.Stream()) * volumeKnob2Level;
        samp_out_right += s162f(sampler2Right.Stream()) * volumeKnob2Level;

        samp_out_left += s162f(sampler3Left.Stream()) * volumeKnob3Level;
        samp_out_right += s162f(sampler3Right.Stream()) * volumeKnob3Level;

        samp_out_left += s162f(sampler4Left.Stream()) * volumeKnob4Level;
        samp_out_right += s162f(sampler4Right.Stream()) * volumeKnob4Level;

        samp_out_left += s162f(sampler5Left.Stream()) * volumeKnob5Level;
        samp_out_right += s162f(sampler5Right.Stream()) * volumeKnob5Level;

        samp_out_left += s162f(sampler6Left.Stream()) * volumeKnob6Level;
        samp_out_right += s162f(sampler6Right.Stream()) * volumeKnob6Level;

        samp_out_left += s162f(sampler7Left.Stream()) * volumeKnob7Level;
        samp_out_right += s162f(sampler7Right.Stream()) * volumeKnob7Level;

        samp_out_left += s162f(sampler8Left.Stream()) * volumeKnob8Level;
        samp_out_right += s162f(sampler8Right.Stream()) * volumeKnob8Level;

        out[i]     = samp_out_left *= .25f * mixKnobLevel;
        out[i + 1] = samp_out_right *= .25f * mixKnobLevel;
    }

    led1.Set(1.f * volumeKnob1Level);
    led1.Update();
}

void InitSampler(WavPlayer&  samplerLeft,
                 WavPlayer&  samplerRight,
                 const char* pathLeft,
                 const char* pathRight,
                 int16_t*    bufferLeft,
                 int16_t*    bufferRight)
{
    samplerLeft.Init(pathLeft, bufferLeft, BUFSIZE, 1);
    samplerRight.Init(pathRight, bufferRight, BUFSIZE, 1);
    samplerLeft.SetLooping(true);
    samplerRight.SetLooping(true);
    samplerLeft.Open(0);
    samplerRight.Open(0);
}

void InitSDCard()
{
    // Initialize SD card
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd_cfg.speed = daisy::SdmmcHandler::Speed::FAST;
    sd_cfg.width = daisy::SdmmcHandler::BusWidth::BITS_1;
    sdcard.Init(sd_cfg);
    fsi.Init(FatFSInterface::Config::MEDIA_SD);

    hardware.Print("mount...");
    if(FRESULT result = f_mount(&fsi.GetSDFileSystem(), fsi.GetSDPath(), 0))
    {
        hardware.PrintLine("did not mount - err %d", result);
        asm("bkpt 255");
    }
    hardware.PrintLine("ok!");
}

void InitPotsAndLED()
{
    // Initialize controls and led
    float            samplerate = hardware.AudioSampleRate(); // per second
    AdcChannelConfig adcConfig
        [NUMBER_OF_ADC_CHANNELS]; // Create an array of AdcChannelConfig
    led1.Init(seed::A6, false, samplerate / 48.f); // red LED

    adcConfig[0].InitSingle(seed::A0); // mix potentiometer

    // One channel configured for 8 inputs via CD4051 mux.
    adcConfig[1].InitMux(seed::A1, 8, seed::D7, seed::D8, seed::D9);

    hardware.adc.Init(adcConfig, NUMBER_OF_ADC_CHANNELS);
    hardware.adc.Start();
}

void BufferSamplers()
{
    sampler1Left.Prepare();
    sampler1Right.Prepare();
    sampler2Left.Prepare();
    sampler2Right.Prepare();
    sampler3Left.Prepare();
    sampler3Right.Prepare();
    sampler4Left.Prepare();
    sampler4Right.Prepare();
    sampler5Left.Prepare();
    sampler5Right.Prepare();
    sampler6Left.Prepare();
    sampler6Right.Prepare();
    sampler7Left.Prepare();
    sampler7Right.Prepare();
    sampler8Left.Prepare();
    sampler8Right.Prepare();
}

int main(void)
{
    hardware.Init();
    hardware.SetAudioBlockSize(256);
    // hardware.StartLog(true);

    InitSDCard();

    // Sampler 1 - Rain - rain-light, (rain-heavy), (rain-roof)
    InitSampler(sampler1Left,
                sampler1Right,
                "0:/rain/rain-light/left",
                "0:/rain/rain-light/right",
                sample1BufferLeft,
                sample1BufferRight);
    // Sampler 2 - Storm - storm
    InitSampler(sampler2Left,
                sampler2Right,
                "0:/storm/storm/left",
                "0:/storm/storm/right",
                sample2BufferLeft,
                sample2BufferRight);
    // Sampler 3 - Noise - (wind), noise/stone-factory
    InitSampler(sampler3Left,
                sampler3Right,
                "0:/noise/stone-factory/left",
                "0:/noise/stone-factory/right",
                sample3BufferLeft,
                sample3BufferRight);
    // Sampler 4 - Birds - birds-dutch, (birds-nz)
    InitSampler(sampler4Left,
                sampler4Right,
                "0:/birds/birds-dutch/left",
                "0:/birds/birds-dutch/right",
                sample4BufferLeft,
                sample4BufferRight);
    // Sampler 5 - Crickets - crickets-dutch, cicada, cat-purring
    InitSampler(sampler5Left,
                sampler5Right,
                "0:/crickets/crickets-dutch/left",
                "0:/crickets/crickets-dutch/right",
                sample5BufferLeft,
                sample5BufferRight);
    // Sampler 6 - Near-water animals - seagulls, frogs, (ducks), oystercatchers
    InitSampler(sampler6Left,
                sampler6Right,
                "0:/near-water-animals/seagulls/left",
                "0:/near-water-animals/seagulls/right",
                sample6BufferLeft,
                sample6BufferRight);
    // Sampler 7 - Water streams - stream-light, stream-heavy, (waterfall)
    InitSampler(sampler7Left,
                sampler7Right,
                "0:/water-streams/stream-light/left",
                "0:/water-streams/stream-light/right",
                sample7BufferLeft,
                sample7BufferRight);
    // Sampler 8 - Waves - waves-river, (waves-sea), (rattling-boatlines)
    InitSampler(sampler8Left,
                sampler8Right,
                "0:/waves/waves-river/left",
                "0:/waves/waves-river/right",
                sample8BufferLeft,
                sample8BufferRight);

    InitPotsAndLED();

    // Start audio
    hardware.Print("starting audio...");
    hardware.StartAudio(AudioCallback);
    hardware.PrintLine("ok!");

    while(true)
    {
        BufferSamplers();
    }
}