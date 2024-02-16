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

Led led1;

// define buffers
// these are initialized globally on the SDRAM memory sector
#define BUFSIZE 4096
int16_t DSY_SDRAM_BSS sample1BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample1BufferRight[BUFSIZE];
int16_t DSY_SDRAM_BSS sample2BufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS sample2BufferRight[BUFSIZE];

#define SAMPLE_1_ENABLED true
#define SAMPLE_2_ENABLED true
#define SAMPLE_3_ENABLED false
#define SAMPLE_4_ENABLED false
#define SAMPLE_5_ENABLED false
#define SAMPLE_6_ENABLED false
#define SAMPLE_7_ENABLED false
#define SAMPLE_8_ENABLED false

constexpr int NUMBER_OF_ADC_CHANNELS = 3;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float samp_out_left  = 0.0f;
    float samp_out_right = 0.0f;

    float mixKnobLevel     = hardware.adc.GetFloat(0);
    float volumeKnob1Level = hardware.adc.GetFloat(1);
    float volumeKnob2Level = hardware.adc.GetFloat(2);

    for(size_t i = 0; i < size; i += 2)
    {
        if(SAMPLE_1_ENABLED)
        {
            // Waves
            float left  = s162f(sampler1Left.Stream());
            float right = s162f(sampler1Right.Stream());
            left *= volumeKnob1Level;  // Apply volume level
            right *= volumeKnob1Level; // Apply volume level
            left *= 0.6f;              // Normalisation
            right *= 0.6f;             // Normalisation
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_2_ENABLED)
        {
            // Birds
            float normalize = 1.2f;
            float left      = s162f(sampler2Left.Stream());
            float right     = s162f(sampler2Right.Stream());
            left *= volumeKnob2Level;  // Apply volume level
            right *= volumeKnob2Level; // Apply volume level
            left *= normalize;         // Normalisation
            right *= normalize;        // Normalisation
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_3_ENABLED)
        {
            float left  = s162f(sampler3Left.Stream());
            float right = s162f(sampler3Right.Stream());
            // left *= volumeKnob1Level;  // Apply volume level
            // right *= volumeKnob1Level; // Apply volume level
            left *= 0.6f;  // Normalisation
            right *= 0.6f; // Normalisation
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_4_ENABLED)
        {
            float left  = s162f(sampler4Left.Stream());
            float right = s162f(sampler4Right.Stream());
            // left *= volumeKnob1Level;  // Apply volume level
            // right *= volumeKnob1Level; // Apply volume level
            left *= 0.6f;  // Normalisation
            right *= 0.6f; // Normalisation
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_5_ENABLED)
        {
            float left  = s162f(sampler5Left.Stream());
            float right = s162f(sampler5Right.Stream());
            // left *= volumeKnob1Level;  // Apply volume level
            // right *= volumeKnob1Level; // Apply volume level
            left *= 0.6f;  // Normalisation
            right *= 0.6f; // Normalisation
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_6_ENABLED)
        {
            float left  = s162f(sampler6Left.Stream());
            float right = s162f(sampler6Right.Stream());
            // left *= volumeKnob1Level;  // Apply volume level
            // right *= volumeKnob1Level; // Apply volume level
            left *= 0.6f;  // Normalisation
            right *= 0.6f; // Normalisation
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_7_ENABLED)
        {
            float left  = s162f(sampler7Left.Stream());
            float right = s162f(sampler7Right.Stream());
            // left *= volumeKnob1Level;  // Apply volume level
            // right *= volumeKnob1Level; // Apply volume level
            left *= 0.6f;  // Normalisation
            right *= 0.6f; // Normalisation
            samp_out_left += left;
            samp_out_right += right;
        }
        if(SAMPLE_8_ENABLED)
        {
            float left  = s162f(sampler8Left.Stream());
            float right = s162f(sampler8Right.Stream());
            // left *= volumeKnob1Level;  // Apply volume level
            // right *= volumeKnob1Level; // Apply volume level
            left *= 0.6f;  // Normalisation
            right *= 0.6f; // Normalisation
            samp_out_left += left;
            samp_out_right += right;
        }
        out[i]     = samp_out_left *= .25f * mixKnobLevel;
        out[i + 1] = samp_out_right *= .25f * mixKnobLevel;
    }

    led1.Set(1.f * volumeKnob1Level);
    led1.Update();
}

int main(void)
{
    hardware.Init();
    hardware.SetAudioBlockSize(24);
    // hardware.StartLog(true);

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

    // Sample 1 - Waves
    sampler1Left.Init("0:/waves-left", sample1BufferLeft, BUFSIZE, 1);
    sampler1Right.Init("0:/waves-right", sample1BufferRight, BUFSIZE, 1);
    sampler1Left.SetLooping(true);
    sampler1Right.SetLooping(true);
    sampler1Left.Open(0);
    sampler1Right.Open(0);

    // Sample 2 - Birds
    sampler2Left.Init("0:/birds-left", sample2BufferLeft, BUFSIZE, 1);
    sampler2Right.Init("0:/birds-right", sample2BufferRight, BUFSIZE, 1);
    sampler2Left.SetLooping(true);
    sampler2Right.SetLooping(true);
    sampler2Left.Open(0);
    sampler2Right.Open(0);

    // Initialize controls and led
    float            samplerate = hardware.AudioSampleRate(); // per second
    AdcChannelConfig adcConfig
        [NUMBER_OF_ADC_CHANNELS]; // Create an array of AdcChannelConfig
    led1.Init(hardware.GetPin(20), false, samplerate / 48.f); // red LED

    adcConfig[1].InitSingle(hardware.GetPin(21)); // sample1 potentiometer
    adcConfig[2].InitSingle(hardware.GetPin(19)); // sample2 potentiometer
    adcConfig[0].InitSingle(hardware.GetPin(18)); // mix potentiometer

    hardware.adc.Init(adcConfig,
                      NUMBER_OF_ADC_CHANNELS); // Has to be >8 channels later
    hardware.adc.Start();

    // Start audio
    hardware.Print("starting audio...");
    hardware.StartAudio(AudioCallback);
    hardware.PrintLine("ok!");

    // Loop forever
    for(;;)
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
}