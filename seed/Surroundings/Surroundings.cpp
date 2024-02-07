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

Switch button1;
Led    led1;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    //Nobody likes a bouncy button
    button1.Debounce();

    // Set in and loop gain from CV_1 and CV_2 respectively
    // float in_level         = 1.f; //patch.GetAdcValue(CV_1);
    float volumeKnob1Level = hardware.adc.GetFloat(0);

    //if you press the button1, toggle the record state
    if(button1.RisingEdge()) {}

    // if you hold the button1 longer than 1000 ms (1 sec), clear the loop
    if(button1.TimeHeldMs() >= 1000.f) {}

    for(size_t i = 0; i < size; i += 2)
    {
        // For now it is mono to both outputs
        float addedLoops = 0.0f;

        float sampler1Output = s162f(sampler1.Stream());
        /* float sampler2Output = s162f(sampler2.Stream()) * 0.5f;
        float sampler3Output = s162f(sampler3.Stream()) * 0.5f;
        float sampler4Output = s162f(sampler4.Stream()) * 0.5f;
        float sampler5Output = s162f(sampler5.Stream()) * 0.5f;
        float sampler6Output = s162f(sampler6.Stream()) * 0.5f;
        float sampler7Output = s162f(sampler7.Stream()) * 0.5f;
        float sampler8Output = s162f(sampler8.Stream()) * 0.5f;
        float sampler9Output = s162f(sampler9.Stream()) * 0.5f; */

        // For now it is mono.
        out[i] = out[i + 1] = sampler1Output;
        // + osc_out;
        /*  + sampler2Output + sampler3Output
                              + sampler4Output + sampler5Output + sampler6Output
                              + sampler7Output + sampler8Output
                              + sampler9Output + osc_out */
    }

    led1.Set(3.f * volumeKnob1Level);
    led1.Update();
}

int main(void)
{
    // Configure and Initialize the Daisy Seed
    // These are separate to allow reconfiguration of any of the internal
    // components before initialization.
    hardware.Configure();
    hardware.Init();
    hardware.SetAudioBlockSize(4);

    // hardware.PrintLine("INITIALIZING...");
    // printf("TEST123");

    // CONFIGURE SD CARD
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    //sd_cfg.speed = daisy::SdmmcHandler::Speed::SLOW;
    sd_cfg.width = daisy::SdmmcHandler::BusWidth::BITS_1;
    sdcard.Init(sd_cfg);
    fsi.Init(FatFSInterface::Config::MEDIA_SD);
    f_mount(&fsi.GetSDFileSystem(), "/", 1);

    // CONFIGURE SAMPLER
    sampler1.Init(fsi.GetSDPath());
    /* sampler2.Init(fsi.GetSDPath());

    sampler1.SetLooping(true);
    sampler2.SetLooping(true);
    sampler3.SetLooping(true);
    sampler4.SetLooping(true);
    sampler5.SetLooping(true);
    sampler6.SetLooping(true);
    sampler7.SetLooping(true);
    sampler8.SetLooping(true);
    sampler9.SetLooping(true); */

    /* sampler1.Open(0);
    sampler9.Open(8); */

    // Close all but the first for now
    /* sampler2.Close();
    sampler9.Close(); */

    float samplerate = hardware.AudioSampleRate(); // per second

    // INITIALIZE CONTROLS AND LEDS
    AdcChannelConfig adcConfig;
    adcConfig.InitSingle(hardware.GetPin(21));                // potentiometer
    led1.Init(hardware.GetPin(20), false, samplerate / 48.f); // red LED
    // button1.Init(hardware.GetPin(28), samplerate / 48.f);     // record button

    hardware.adc.Init(&adcConfig, 1);
    hardware.adc.Start();

    // Start the audio callback
    hardware.StartAudio(AudioCallback);

    // Loop forever
    for(;;)
    {
        sampler1.Prepare();
        /* sampler2.Prepare();
        sampler3.Prepare();
        sampler4.Prepare();
        sampler5.Prepare();
        sampler6.Prepare();
        sampler7.Prepare();
        sampler8.Prepare();
        sampler9.Prepare(); */
    }
}
}