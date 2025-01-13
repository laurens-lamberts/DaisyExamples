#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "daisysp.h"
// #include "sys/fatfs.h"
#include "modules/samplevoice/samplevoice.cpp"

// Wav files need to be 16 bit - 1 channel (though 2 channels seems fine?) - 48kHz

using namespace daisy;
using namespace daisysp;

DaisySeed      hardware;
SdmmcHandler   sdcard;
FatFSInterface fsi;
SampleVoice    sampler1Left;
SampleVoice    sampler1Right;
SampleVoice    sampler2Left;
SampleVoice    sampler2Right;
SampleVoice    sampler3Left;
SampleVoice    sampler3Right;
SampleVoice    sampler4Left;
SampleVoice    sampler4Right;
SampleVoice    sampler5Left;
SampleVoice    sampler5Right;
SampleVoice    sampler6Left;
SampleVoice    sampler6Right;
SampleVoice    sampler7Left;
SampleVoice    sampler7Right;
SampleVoice    sampler8Left;
SampleVoice    sampler8Right;

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
    float r, g, b;

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
        samp_out_left += sampler1Left.Stream() * volumeKnob1Level;
        samp_out_right += sampler1Right.Stream() * volumeKnob1Level;

        samp_out_left += sampler2Left.Stream() * volumeKnob2Level;
        samp_out_right += sampler2Right.Stream() * volumeKnob2Level;

        samp_out_left += sampler3Left.Stream() * volumeKnob3Level;
        samp_out_right += sampler3Right.Stream() * volumeKnob3Level;

        samp_out_left += sampler4Left.Stream() * volumeKnob4Level;
        samp_out_right += sampler4Right.Stream() * volumeKnob4Level;

        samp_out_left += sampler5Left.Stream() * volumeKnob5Level;
        samp_out_right += sampler5Right.Stream() * volumeKnob5Level;

        samp_out_left += sampler6Left.Stream() * volumeKnob6Level;
        samp_out_right += sampler6Right.Stream() * volumeKnob6Level;

        samp_out_left += sampler7Left.Stream() * volumeKnob7Level;
        samp_out_right += sampler7Right.Stream() * volumeKnob7Level;

        samp_out_left += sampler8Left.Stream() * volumeKnob8Level;
        samp_out_right += sampler8Right.Stream() * volumeKnob8Level;

        out[i]     = samp_out_left *= .25f * mixKnobLevel;
        out[i + 1] = samp_out_right *= .25f * mixKnobLevel;
    }

    // Debounce the button
    button1.Debounce();

    led1.Set(1.f * volumeKnob1Level);
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

void InitSampler(SampleVoice& samplerLeft,
                 SampleVoice& samplerRight,
                 const char*  pathLeft,
                 const char*  pathRight)
{
    hardware.PrintLine(
        "Initializing sampler with paths: %s, %s", pathLeft, pathRight);
    uint32_t bufferSize = 48000; // Adjust buffer size as needed
    samplerLeft.Init(bufferSize);
    // samplerRight.Init(bufferSize);
    if(samplerLeft.SetSample(pathLeft) != 0)
    {
        hardware.PrintLine("Failed to set sample for samplerLeft");
    }
    // if(samplerRight.SetSample(pathRight) != 0)
    // {
    //     hardware.PrintLine("Failed to set sample for samplerRight");
    // }
    samplerLeft.Play();
    // samplerRight.Play();
}

void InitSDCard()
{
    // Initialize SD card
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd_cfg.speed = daisy::SdmmcHandler::Speed::SLOW;
    sd_cfg.width = daisy::SdmmcHandler::BusWidth::BITS_1;
    sdcard.Init(sd_cfg);
    fsi.Init(FatFSInterface::Config::MEDIA_SD);

    hardware.Print("mount...");

    volatile FRESULT result
        = f_mount(&fsi.GetSDFileSystem(), fsi.GetSDPath(), 1);
    if(result != FR_OK)
    {
        hardware.PrintLine("did not mount - err %d", result);
        asm("bkpt 255");
    }
    hardware.PrintLine("ok!");
}

void InitPotsButtonsAndLED()
{
    // Initialize controls and led
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
    hardware.PrintLine("Buffering samplers...");
    // No need to call Prepare() as in WavPlayer
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
                "0:/rain/rain-light/left/rain-left.wav",
                "0:/rain/rain-light/right/rain-left.wav");
    // // Sampler 2 - Storm - storm
    // InitSampler(sampler2Left,
    //             sampler2Right,
    //             "0:/storm/storm/left",
    //             "0:/storm/storm/right");
    // // Sampler 3 - Noise - (wind), noise/stone-factory
    // InitSampler(sampler3Left,
    //             sampler3Right,
    //             "0:/noise/stone-factory/left",
    //             "0:/noise/stone-factory/right");
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
    hardware.Print("starting audio...");
    hardware.StartAudio(AudioCallback);
    hardware.PrintLine("ok!");

    for(;;)
    {
        BufferSamplers();
    }
}