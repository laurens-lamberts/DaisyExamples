// Derived from synthux looper

#include <Daisy.h>
#include <daisysp.h>

using namespace daisy;
using namespace daisysp;

// Setup pins
const DaisySeed::Pin record_pin = DaisySeed::S30;
const DaisySeed::Pin loop_start_pin = DaisySeed::S31;
const DaisySeed::Pin loop_length_pin = DaisySeed::S32;
const DaisySeed::Pin pitch_pin = DaisySeed::S33;

const float kKnobMax = 1023.0f;

// Allocate buffer in SDRAM
const uint32_t kBufferLengthSec = 5;
const uint32_t kSampleRate = 48000;
const size_t kBufferLengthSamples = kBufferLengthSec * kSampleRate;
static float DSY_SDRAM_BSS buffer[kBufferLengthSamples];

static Looper looper;
static PitchShifter pitch_shifter;

void AudioCallback(float *in, float *out, size_t size)
{
  for (size_t i = 0; i < size; i += 2)
  {
    float looper_out = looper.Process(in[i + 1]);
    out[i] = out[i + 1] = pitch_shifter.Process(looper_out);
  }
}

void setup()
{
  DaisySeed hw;
  hw.Configure();
  hw.Init();

  // Setup looper
  looper.Init(buffer, kBufferLengthSamples);

  // Setup pitch shifter
  pitch_shifter.Init(hw.AudioSampleRate());

  // Setup pins
  hw.gpio.Init();
  hw.gpio.Mode(record_pin, GpioHandle::INPUT);

  hw.StartAudio(AudioCallback);
}

void loop()
{
  // Set loop parameters
  float loop_start = AnalogPin(loop_start_pin).Read() / kKnobMax;
  float loop_length = AnalogPin(loop_length_pin).Read() / kKnobMax;
  looper.SetLoop(loop_start, loop_length);

  // Toggle record
  bool record_on = GpioPin(record_pin).Read();
  looper.SetRecording(record_on);

  // Set pitch
  float pitch_val = AnalogPin(pitch_pin).Read() / kKnobMax;
  set_pitch(pitch_val);
}

void set_pitch(float pitch_val)
{
  int pitch = 0;
  // Allow some gap in the middle of the knob turn so
  // it's easy to catch zero position
  if (pitch_val < 0.45f || pitch_val > 0.55f)
  {
    pitch = 12.0f * (pitch_val - 0.5f);
  }
  pitch_shifter.SetTransposition(pitch);
}
