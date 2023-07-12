#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "daisysp.h"

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
WavPlayer      sampler9;

Oscillator osc;

Switch button1;
Led    led1;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float osc_out;

    //Nobody likes a bouncy button
    button1.Debounce();

    // Set in and loop gain from CV_1 and CV_2 respectively
    float in_level   = 1.f; //patch.GetAdcValue(CV_1);
    float loop_level = hardware.adc.GetFloat(0);

    //if you press the button1, toggle the record state
    if(button1.RisingEdge()) {}

    // if you hold the button1 longer than 1000 ms (1 sec), clear the loop
    if(button1.TimeHeldMs() >= 1000.f) {}

    //Convert floating point knob to midi (0-127)
    //Then convert midi to freq. in Hz
    //osc.SetFreq(mtof(hardware.adc.GetFloat(0) * 127));

    for(size_t i = 0; i < size; i += 2)
    {
        osc.SetAmp(loop_level);
        //get the next oscillator sample
        osc_out = osc.Process();

        float sampler1Output = s162f(sampler1.Stream()) * 0.5f;

        // For now it is mono.
        out[i] = out[i + 1] = sampler1Output + osc_out;
    }

    led1.Set(3.f);
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

    // CONFIGURE SD CARD
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sdcard.Init(sd_cfg);
    fsi.Init(FatFSInterface::Config::MEDIA_SD);
    f_mount(&fsi.GetSDFileSystem(), "/", 1);

    // CONFIGURE SAMPLER
    sampler1.Init(fsi.GetSDPath());
    sampler1.SetLooping(true);

    float samplerate = hardware.AudioSampleRate(); // per second

    // INITIALIZE CONTROLS AND LEDS
    AdcChannelConfig adcConfig;
    adcConfig.InitSingle(hardware.GetPin(21));                // potentiometer
    led1.Init(hardware.GetPin(20), false, samplerate / 48.f); // red LED
    button1.Init(hardware.GetPin(28), samplerate / 48.f);     // record button

    hardware.adc.Init(&adcConfig, 1);
    hardware.adc.Start();

    // CONFIGURE OSCILLATOR
    osc.Init(samplerate);
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetAmp(0);
    osc.SetFreq(400);

    // Start the audio callback
    hardware.StartAudio(AudioCallback);

    // Loop forever
    for(;;)
    {
        // Prepare buffers for sampler as needed
        sampler1.Prepare();
    }
}
