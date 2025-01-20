#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "daisysp.h"
#include "wavplayer.h"

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
RgbLed        rgbLed1;
AnalogControl pot1;
Switch        button1;

constexpr int NUMBER_OF_ADC_CHANNELS = 2;

float intToFloat(uint16_t in)
{
    return static_cast<float>(in) / 65536.0f;
}
struct RGB
{
    float r;
    float g;
    float b;
};

RGB hsvToRgb(float h, float s, float v)
{
    float r = 0.0, g = 0.0, b = 0.0;

    int   i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch(i % 6)
    {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return {r, g, b};
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

    // Debounce the button
    button1.Debounce();

    led1.Set(1.f * mixKnobLevel);
    led1.Update();

    if(button1.Pressed())
    {
        // Cycle through presets and show the selected one on the RGB LED
        RGB color = hsvToRgb(volumeKnob1Level, 1.0f, 1.0f);
        rgbLed1.Set(color.r, color.g, color.b);
    }
    if(button1.FallingEdge())
    {
        // TODO: just released. Save selected preset.
    }
    // rgbLed1.Set(0.0f, 0.0f, 0.6f);
    rgbLed1.Update();
}

void InitSampler(WavPlayer&  samplerLeft,
                 WavPlayer&  samplerRight,
                 const char* pathLeft,
                 const char* pathRight)
{
    samplerLeft.Init(pathLeft);
    samplerRight.Init(pathRight);
    samplerLeft.SetLooping(true);
    samplerRight.SetLooping(true);
    samplerLeft.Open(0);
    samplerRight.Open(0);
}

void InitSDCard()
{
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd_cfg.width = daisy::SdmmcHandler::BusWidth::BITS_1;

    if(sdcard.Init(sd_cfg) != daisy::SdmmcHandler::Result::OK)
    {
        return;
    }
    fsi.Init(FatFSInterface::Config::MEDIA_SD);

    hardware.Print("sd card mount...");
    if(FRESULT result = f_mount(&fsi.GetSDFileSystem(), fsi.GetSDPath(), 0))
    {
        hardware.PrintLine("sd card did not mount - err %d", result);
        asm("bkpt 255");
    }
    hardware.PrintLine("sd card mount ok!");
}

void InitPotsButtonsAndLED()
{
    float            samplerate = hardware.AudioSampleRate(); // per second
    AdcChannelConfig adcConfig
        [NUMBER_OF_ADC_CHANNELS]; // Create an array of AdcChannelConfig

    led1.Init(seed::A6, false, samplerate / 48.f); // red LED
    rgbLed1.Init(seed::D11, seed::D12, seed::D13, true);

    button1.Init(seed::A5, 1000); // to be updated at a 1kHz samplerate

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

    InitSDCard();

    // Sampler 1 - Rain - rain-light, (rain-heavy), (rain-roof)
    InitSampler(sampler1Left,
                sampler1Right,
                "0:/rain/rain-light/left",
                "0:/rain/rain-light/right");
    // Sampler 2 - Storm - storm
    InitSampler(sampler2Left,
                sampler2Right,
                "0:/storm/storm/left",
                "0:/storm/storm/right");
    // Sampler 3 - Noise - (wind), noise/stone-factory
    InitSampler(sampler3Left,
                sampler3Right,
                "0:/noise/stone-factory/left",
                "0:/noise/stone-factory/right");
    // // Sampler 4 - Birds - birds-dutch, (birds-nz)
    // InitSampler(sampler4Left,
    //             sampler4Right,
    //             "0:/birds/birds-dutch/left",
    //             "0:/birds/birds-dutch/right");
    // // Sampler 5 - Crickets - crickets-dutch, cicada, cat-purring
    // InitSampler(sampler5Left,
    //             sampler5Right,
    //             "0:/crickets/crickets-dutch/left",
    //             "0:/crickets/crickets-dutch/right");
    // // Sampler 6 - Near-water animals - seagulls, frogs, (ducks), oystercatchers
    // InitSampler(sampler6Left,
    //             sampler6Right,
    //             "0:/near-water-animals/seagulls/left",
    //             "0:/near-water-animals/seagulls/right");
    // // Sampler 7 - Water streams - stream-light, stream-heavy, (waterfall)
    // InitSampler(sampler7Left,
    //             sampler7Right,
    //             "0:/water-streams/stream-light/left",
    //             "0:/water-streams/stream-light/right");
    // // Sampler 8 - Waves - waves-river, (waves-sea), (rattling-boatlines)
    // InitSampler(sampler8Left,
    //             sampler8Right,
    //             "0:/waves/waves-river/left",
    //             "0:/waves/waves-river/right");

    InitPotsButtonsAndLED();

    // Start audio
    hardware.StartAudio(AudioCallback);

    for(;;)
    {
        BufferSamplers();
    }
}