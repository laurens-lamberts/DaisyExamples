#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "daisysp.h"
#include "modules/wavplayer/wavplayer.h"

// Wav files need to be 16 bit - 1 channel (though 2 channels seems fine?) - 48kHz

using namespace daisy;
using namespace daisysp;

DaisySeed      hardware;
SdmmcHandler   sdcard;
FatFSInterface fsi;
WavPlayer      sampler1Left;
WavPlayer      sampler1Right;
WavPlayer      sampler1;
WavPlayer      sampler2;
WavPlayer      sampler3;
WavPlayer      sampler4;
WavPlayer      sampler5;
WavPlayer      sampler6;
WavPlayer      sampler7;
WavPlayer      sampler8;

Led led1;

// define buffers
// these are initialized globally on the SDRAM memory sector
#define BUFSIZE 4096
int16_t DSY_SDRAM_BSS bufferLeft[BUFSIZE];
int16_t DSY_SDRAM_BSS bufferRight[BUFSIZE];

#define SAMPLE_1_ENABLED false
#define SAMPLE_2_ENABLED false
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
    float samp_out       = 0.0f;
    float mixKnobLevel   = hardware.adc.GetFloat(0);

    float volumeKnob1Level = hardware.adc.GetFloat(1);
    float volumeKnob2Level = hardware.adc.GetFloat(2);

    // For now it is mono to both outputs
    for(size_t i = 0; i < size; i += 2)
    {
        // stream samplers
        samp_out_left += s162f(sampler1Left.Stream());
        samp_out_right += s162f(sampler1Right.Stream());

        if(SAMPLE_1_ENABLED)
        {
            // Storm
            float sampler1Output = s162f(sampler1.Stream());
            sampler1Output *= volumeKnob1Level; // Apply volume level
            sampler1Output *= 1.8f;             // Normalisation
            samp_out += sampler1Output;
        }
        if(SAMPLE_2_ENABLED)
        {
            // Waves
            float sampler2Output = s162f(sampler2.Stream());
            sampler2Output *= volumeKnob2Level; // Apply volume level
            sampler2Output *= 0.6f;             // Normalisation
            samp_out += sampler2Output;
        }
        if(SAMPLE_3_ENABLED)
        {
            float sampler3Output = s162f(sampler3.Stream());
            samp_out += sampler3Output;
        }
        if(SAMPLE_4_ENABLED)
        {
            float sampler4Output = s162f(sampler4.Stream());
            samp_out += sampler4Output;
        }
        if(SAMPLE_5_ENABLED)
        {
            float sampler5Output = s162f(sampler5.Stream());
            samp_out += sampler5Output;
        }
        if(SAMPLE_6_ENABLED)
        {
            float sampler6Output = s162f(sampler6.Stream());
            samp_out += sampler6Output;
        }
        if(SAMPLE_7_ENABLED)
        {
            float sampler7Output = s162f(sampler7.Stream());
            samp_out += sampler7Output;
        }
        if(SAMPLE_8_ENABLED)
        {
            float sampler8Output = s162f(sampler8.Stream());
            samp_out += sampler8Output;
        }
        // out[i] = out[i + 1] = samp_out * .25f * mixKnobLevel;
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

    // System::Delay(1000);

    // hardware.PrintLine("INITIALIZING...");
    // printf("TEST123");
    // fflush(stdout);

    // Initialize SD card
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    // sd_cfg.speed = daisy::SdmmcHandler::Speed::FAST;
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
    // System::Delay(1000);

    // Initialize first sampler on /waves-left
    if(int result = sampler1Left.Init("0:/waves-left", bufferLeft, BUFSIZE, 1))
    {
        hardware.PrintLine("did not initialize sampler1Left - err %d", result);
        asm("bkpt 255");
    }
    hardware.PrintLine("sampler1Left - loaded %d files",
                       sampler1Left.GetNumberFiles());
    // System::Delay(500);

    // Initialize second sampler on /waves-right
    if(int result
       = sampler1Right.Init("0:/waves-right", bufferRight, BUFSIZE, 1))
    {
        hardware.PrintLine("did not initialize sampler1Right - err %d", result);
        asm("bkpt 255");
    }
    hardware.PrintLine("sampler1Right - loaded %d files",
                       sampler1Right.GetNumberFiles());
    // System::Delay(500);
    // f_mount(&fsi.GetSDFileSystem(), "/", 1);

    std::string sdPath = fsi.GetSDPath();

    // Set samplers to looping
    sampler1Left.SetLooping(true);
    sampler1Right.SetLooping(true);

    // Open audio files in samplers
    sampler1Left.Open(0);
    sampler1Right.Open(0);

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

    /* if(SAMPLE_1_ENABLED)
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
    } */


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
        // prepare samples for sampler 1; manage errors
        if(int result = sampler1Left.Prepare())
        {
            hardware.PrintLine("ERROR WHILE PREPARING sampler1Left - err %d",
                               result);
            asm("bkpt 255");
        }

        // prepare samples for sampler 2; manage errors
        if(int result = sampler1Right.Prepare())
        {
            hardware.PrintLine("ERROR WHILE PREPARING sampler1Right - err %d",
                               result);
            asm("bkpt 255");
        }

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