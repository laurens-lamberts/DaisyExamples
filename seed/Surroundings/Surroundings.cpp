#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "daisysp.h"

// Wav files need to be 16 bit - 1 channel (though 2 channels seems fine?) - 48kHz

using namespace daisy;
using namespace daisysp;

DaisySeed      hardware;
SdmmcHandler   sdcard;
FatFSInterface fsi;
WavPlayer      sampler1;
WavPlayer      sampler2;
WavPlayer      sampler3;
WavPlayer      sampler4;
WavPlayer      sampler5;
WavPlayer      sampler6;
WavPlayer      sampler7;
WavPlayer      sampler8;

Led led1;

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
    float mixKnobLevel = hardware.adc.GetFloat(0);

    float volumeKnob1Level = hardware.adc.GetFloat(1);
    float volumeKnob2Level = hardware.adc.GetFloat(2);

    // For now it is mono to both outputs
    for(size_t i = 0; i < size; i += 2)
    {
        // out[i] = out[i + 1] = 0.0f;
        if(SAMPLE_1_ENABLED)
        {
            // Storm
            float sampler1Output = s162f(sampler1.Stream());
            sampler1Output *= volumeKnob1Level; // Apply volume level
            sampler1Output *= 1.8f;             // Normalisation
            out[i] = out[i + 1] = sampler1Output;
        }
        if(SAMPLE_2_ENABLED)
        {
            // Waves
            float sampler2Output = s162f(sampler2.Stream());
            sampler2Output *= volumeKnob2Level; // Apply volume level
            sampler2Output *= 0.6f;             // Normalisation
            out[i] += out[i + 1] += sampler2Output;
        }
        if(SAMPLE_3_ENABLED)
        {
            float sampler3Output = s162f(sampler3.Stream());
            out[i] += out[i + 1] += sampler3Output;
        }
        if(SAMPLE_4_ENABLED)
        {
            float sampler4Output = s162f(sampler4.Stream());
            out[i] += out[i + 1] += sampler4Output;
        }
        if(SAMPLE_5_ENABLED)
        {
            float sampler5Output = s162f(sampler5.Stream());
            out[i] += out[i + 1] += sampler5Output;
        }
        if(SAMPLE_6_ENABLED)
        {
            float sampler6Output = s162f(sampler6.Stream());
            out[i] += out[i + 1] += sampler6Output;
        }
        if(SAMPLE_7_ENABLED)
        {
            float sampler7Output = s162f(sampler7.Stream());
            out[i] += out[i + 1] += sampler7Output;
        }
        if(SAMPLE_8_ENABLED)
        {
            float sampler8Output = s162f(sampler8.Stream());
            out[i] += out[i + 1] += sampler8Output;
        }
        out[i] *= mixKnobLevel;
        out[i + 1] *= mixKnobLevel;
    }

    led1.Set(1.f * volumeKnob1Level);
    led1.Update();
}

int main(void)
{
    // Configure and Initialize the Daisy Seed
    // These are separate to allow reconfiguration of any of the internal
    // components before initialization.
    // hardware.Configure();
    hardware.Init();
    hardware.SetAudioBlockSize(4);

    // hardware.PrintLine("INITIALIZING...");
    // printf("TEST123");
    // fflush(stdout);

    // CONFIGURE SD CARD
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    // sd_cfg.speed = daisy::SdmmcHandler::Speed::FAST;
    sd_cfg.width = daisy::SdmmcHandler::BusWidth::BITS_1;
    sdcard.Init(sd_cfg);
    fsi.Init(FatFSInterface::Config::MEDIA_SD);
    f_mount(&fsi.GetSDFileSystem(), "/", 1);

    std::string sdPath = fsi.GetSDPath();

    // CONFIGURE SAMPLER
    // 1: birds-dutch
    // 2: cat purring
    // 3: crickets
    // 4: crickets?
    // 5: stream A
    // 6: oystercatcher
    // 7: stream B
    // 8: stream C ?
    // 9: stream
    // 10: stream
    // 11: stream
    // 12: stream

    if(SAMPLE_1_ENABLED)
    {
        sampler1.Init(sdPath.c_str());
        sampler1.SetLooping(true);
        sampler1.Open(1); //closest knob
    }
    if(SAMPLE_2_ENABLED)
    {
        sampler2.Init(sdPath.c_str());
        sampler2.SetLooping(true);
        sampler2.Open(3); //farthest knob
    }
    if(SAMPLE_3_ENABLED)
    {
        sampler3.Init(sdPath.c_str());
        sampler3.SetLooping(true);
        sampler3.Open(1);
    }
    if(SAMPLE_4_ENABLED)
    {
        sampler4.Init(sdPath.c_str());
        sampler4.SetLooping(true);
        sampler4.Open(3);
    }
    if(SAMPLE_5_ENABLED)
    {
        sampler5.Init(sdPath.c_str());
        sampler5.SetLooping(true);
        sampler5.Open(1);
    }
    if(SAMPLE_6_ENABLED)
    {
        sampler6.Init(sdPath.c_str());
        sampler6.SetLooping(true);
        sampler6.Open(3);
    }
    if(SAMPLE_7_ENABLED)
    {
        sampler7.Init(sdPath.c_str());
        sampler7.SetLooping(true);
        sampler7.Open(1);
    }
    if(SAMPLE_8_ENABLED)
    {
        sampler8.Init(sdPath.c_str());
        sampler8.SetLooping(true);
        sampler8.Open(3);
    }

    float samplerate = hardware.AudioSampleRate(); // per second

    // INITIALIZE CONTROLS AND LEDS
    AdcChannelConfig adcConfig
        [NUMBER_OF_ADC_CHANNELS]; // Create an array of AdcChannelConfig
    led1.Init(hardware.GetPin(20), false, samplerate / 48.f); // red LED

    adcConfig[1].InitSingle(hardware.GetPin(21)); // sample1 potentiometer
    adcConfig[2].InitSingle(hardware.GetPin(19)); // sample2 potentiometer
    adcConfig[0].InitSingle(hardware.GetPin(18)); // mix potentiometer

    hardware.adc.Init(adcConfig,
                      NUMBER_OF_ADC_CHANNELS); // Has to be >8 channels later
    hardware.adc.Start();

    // Start the audio callback
    hardware.StartAudio(AudioCallback);

    // Loop forever
    for(;;)
    {
        if(SAMPLE_1_ENABLED)
        {
            sampler1.Prepare();
        }
        if(SAMPLE_2_ENABLED)
        {
            sampler2.Prepare();
        }
        if(SAMPLE_3_ENABLED)
        {
            sampler3.Prepare();
        }
        if(SAMPLE_4_ENABLED)
        {
            sampler4.Prepare();
        }
        if(SAMPLE_5_ENABLED)
        {
            sampler5.Prepare();
        }
        if(SAMPLE_6_ENABLED)
        {
            sampler6.Prepare();
        }
        if(SAMPLE_7_ENABLED)
        {
            sampler7.Prepare();
        }
        if(SAMPLE_8_ENABLED)
        {
            sampler8.Prepare();
        }
    }
}