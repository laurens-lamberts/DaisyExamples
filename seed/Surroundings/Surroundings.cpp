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
#define BUFSIZE 4096
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

#define SAMPLE_1_ENABLED true
#define SAMPLE_2_ENABLED true
#define SAMPLE_3_ENABLED true
#define SAMPLE_4_ENABLED false
#define SAMPLE_5_ENABLED false
#define SAMPLE_6_ENABLED false
#define SAMPLE_7_ENABLED false
#define SAMPLE_8_ENABLED false

constexpr int NUMBER_OF_ADC_CHANNELS = 4;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float samp_out_left  = 0.0f;
    float samp_out_right = 0.0f;

    float mixKnobLevel     = hardware.adc.GetFloat(0);
    float volumeKnob1Level = hardware.adc.GetFloat(1);
    float volumeKnob2Level = hardware.adc.GetFloat(2);
    float volumeKnob3Level = hardware.adc.GetFloat(3);
    float volumeKnob4Level = 0.1f;
    float volumeKnob5Level = 0.1f;
    float volumeKnob6Level = 0.1f;
    float volumeKnob7Level = 0.1f;
    float volumeKnob8Level = 0.1f;

    for(size_t i = 0; i < size; i += 2)
    {
        if(SAMPLE_1_ENABLED)
        {
            float left  = s162f(sampler1Left.Stream());
            float right = s162f(sampler1Right.Stream());
            left *= volumeKnob1Level;  // Apply volume level
            right *= volumeKnob1Level; // Apply volume level
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_2_ENABLED)
        {
            float left  = s162f(sampler2Left.Stream());
            float right = s162f(sampler2Right.Stream());
            left *= volumeKnob2Level;  // Apply volume level
            right *= volumeKnob2Level; // Apply volume level
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_3_ENABLED)
        {
            float left  = s162f(sampler3Left.Stream());
            float right = s162f(sampler3Right.Stream());
            left *= volumeKnob3Level;  // Apply volume level
            right *= volumeKnob3Level; // Apply volume level
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_4_ENABLED)
        {
            float left  = s162f(sampler4Left.Stream());
            float right = s162f(sampler4Right.Stream());
            left *= volumeKnob4Level;  // Apply volume level
            right *= volumeKnob4Level; // Apply volume level
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_5_ENABLED)
        {
            float left  = s162f(sampler5Left.Stream());
            float right = s162f(sampler5Right.Stream());
            left *= volumeKnob5Level;  // Apply volume level
            right *= volumeKnob5Level; // Apply volume level
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_6_ENABLED)
        {
            float left  = s162f(sampler6Left.Stream());
            float right = s162f(sampler6Right.Stream());
            left *= volumeKnob6Level;  // Apply volume level
            right *= volumeKnob6Level; // Apply volume level
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_7_ENABLED)
        {
            float left  = s162f(sampler7Left.Stream());
            float right = s162f(sampler7Right.Stream());
            left *= volumeKnob7Level;  // Apply volume level
            right *= volumeKnob7Level; // Apply volume level
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_8_ENABLED)
        {
            float left  = s162f(sampler8Left.Stream());
            float right = s162f(sampler8Right.Stream());
            left *= volumeKnob8Level;  // Apply volume level
            right *= volumeKnob8Level; // Apply volume level
            samp_out_left += left;
            samp_out_right += right;
        }
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
    led1.Init(hardware.GetPin(21), false, samplerate / 48.f); // red LED

    // pot1.Init(hardware.adc.GetPtr(0), samplerate, false, false, 0.002f);

    adcConfig[0].InitSingle(hardware.GetPin(15)); // mix potentiometer
    adcConfig[1].InitSingle(hardware.GetPin(16)); // sample1 potentiometer
    adcConfig[2].InitSingle(hardware.GetPin(17)); // sample2 potentiometer
    adcConfig[3].InitSingle(hardware.GetPin(18)); // sample3 potentiometer

    hardware.adc.Init(adcConfig,
                      NUMBER_OF_ADC_CHANNELS); // Has to be >8 channels later
    hardware.adc.Start();
}

void BufferSamplers()
{
    if(SAMPLE_1_ENABLED)
    {
        sampler1Left.Prepare();
        sampler1Right.Prepare();
    }
    if(SAMPLE_2_ENABLED)
    {
        sampler2Left.Prepare();
        sampler2Right.Prepare();
    }
    if(SAMPLE_3_ENABLED)
    {
        sampler3Left.Prepare();
        sampler3Right.Prepare();
    }
    if(SAMPLE_4_ENABLED)
    {
        sampler4Left.Prepare();
        sampler4Right.Prepare();
    }
    if(SAMPLE_5_ENABLED)
    {
        sampler5Left.Prepare();
        sampler5Right.Prepare();
    }
    if(SAMPLE_6_ENABLED)
    {
        sampler6Left.Prepare();
        sampler6Right.Prepare();
    }
    if(SAMPLE_7_ENABLED)
    {
        sampler7Left.Prepare();
        sampler7Right.Prepare();
    }
    if(SAMPLE_8_ENABLED)
    {
        sampler8Left.Prepare();
        sampler8Right.Prepare();
    }
}

int main(void)
{
    hardware.Init();
    hardware.SetAudioBlockSize(24);
    // hardware.StartLog(true);

    InitSDCard();

    // Sampler 1 - Waves
    InitSampler(sampler1Left,
                sampler1Right,
                "0:/waves/waves-river/left",
                "0:/waves/waves-river/right",
                sample1BufferLeft,
                sample1BufferRight);
    // Sampler 2 - Birds
    InitSampler(sampler2Left,
                sampler2Right,
                "0:/birds/birds-dutch/left",
                "0:/birds/birds-dutch/right",
                sample2BufferLeft,
                sample2BufferRight);
    // Sampler 3 - Rain
    InitSampler(sampler3Left,
                sampler3Right,
                "0:/rain/rain-light/left",
                "0:/rain/rain-light/right",
                sample3BufferLeft,
                sample3BufferRight);

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