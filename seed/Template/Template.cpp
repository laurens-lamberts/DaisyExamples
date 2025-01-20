#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed     hardware;
Led           led1;
RgbLed        rgbLed1;
AnalogControl pot1;
Switch        button1;

ReverbSc verb;
// Bitcrush bitcrush;
Chorus chorus;

Svf filterLeft, filterRight;

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
    float r = 0.0f, g = 0.0f, b = 0.0f;

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
    // float mixKnobLevel = hardware.adc.GetFloat(0);
    float rgbCycleKnobLevel = intToFloat(hardware.adc.GetMux(1, 0));
    float dryLevel          = intToFloat(hardware.adc.GetMux(1, 1));
    float filterCutoff      = intToFloat(hardware.adc.GetMux(1, 2));
    // float multiplexerKnobLevel = intToFloat(hardware.adc.GetMux(1, 1));
    // float adcKnobLevel = hardware.adc.GetFloat(3);

    for(size_t i = 0; i < size; i += 2)
    {
        // bitcrush.SetBitDepth(depth + 2);
        // bitcrush.SetCrushRate((depth + 2) * 2000);

        float dryLeft  = in[i];
        float dryRight = in[i + 1];

        filterLeft.SetFreq(filterCutoff * 10000);
        filterLeft.Process(dryLeft);
        filterRight.SetFreq(filterCutoff * 10000);
        filterRight.Process(dryLeft);

        // float crushedLeft  = bitcrush.Process(dryLeft);
        // float crushedRight = bitcrush.Process(dryRight);

        // verb.Process(in[i],
        //              crushedRight,
        //              &out[i],
        //              &out[i + 1]); // TODO; decide, series or parallel?

        float filteredLeft  = filterLeft.High();
        float filteredRight = filterRight.High();

        chorus.Process(filteredLeft);

        float chorusedLeft  = chorus.GetLeft();
        float chorusedRight = chorus.GetRight();

        // padding verb, beneath the playing
        verb.Process(chorusedLeft, chorusedRight, &out[i], &out[i + 1]);

        out[i] += chorusedLeft + (dryLeft * dryLevel);       // * mixKnobLevel;
        out[i + 1] += chorusedRight + (dryRight * dryLevel); // * mixKnobLevel;
    }

    // Debounce the button
    button1.Debounce();

    led1.Set(1.f);
    led1.Update();

    RGB color = hsvToRgb(rgbCycleKnobLevel, 1.0f, 1.0f);
    rgbLed1.Set(color.r, color.g, color.b);
    rgbLed1.Update();
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

int main(void)
{
    hardware.Init();
    hardware.SetAudioBlockSize(4);
    // hardware.StartLog(true);

    float sample_rate = hardware.AudioSampleRate();

    InitPotsButtonsAndLED();

    // setup reverb
    verb.Init(sample_rate);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(sample_rate / 8);

    // Initialize Filter, and set parameters.
    filterLeft.Init(sample_rate);
    filterLeft.SetFreq(500.0);
    filterLeft.SetRes(0.2);
    filterLeft.SetDrive(0.8);
    filterRight.Init(sample_rate);
    filterRight.SetFreq(500.0);
    filterRight.SetRes(0.2);
    filterRight.SetDrive(0.8);

    chorus.Init(sample_rate);
    chorus.SetLfoFreq(0.5f, 0.5f);
    chorus.SetLfoDepth(1.f, 1.f);
    chorus.SetDelay(0.2f, 0.2f);
    chorus.SetFeedback(0.0f, 0.0f);
    // chorus.SetDelay(.75f, .9f);

    // // setup bitcrusher
    // bitcrush.Init(sample_rate);
    // bitcrush.SetBitDepth(6);
    // bitcrush.SetCrushRate(10000);

    // Start audio
    hardware.Print("starting audio...");
    hardware.StartAudio(AudioCallback);
    hardware.PrintLine("ok!");

    for(;;) {}
}