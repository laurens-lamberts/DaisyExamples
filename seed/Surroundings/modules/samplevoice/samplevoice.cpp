#include <stdio.h>
#include <string.h>
#include "sys/fatfs.h"
#include "daisy_seed.h"      // Include the Daisy library
#include "util/wav_format.h" // Include the WAV format header

using namespace daisy;

typedef struct
{
    uint32_t ChunkID;       // "RIFF"
    uint32_t ChunkSize;     // Size of the file
    uint32_t Format;        // "WAVE"
    uint32_t SubChunk1ID;   // "fmt "
    uint32_t SubChunk1Size; // Size of the fmt chunk
    uint16_t AudioFormat;   // Audio format (1 for PCM)
    uint16_t NbrChannels;   // Number of channels
    uint32_t SampleRate;    // Sample rate
    uint32_t ByteRate;      // Byte rate
    uint16_t BlockAlign;    // Block align
    uint16_t BitPerSample;  // Bits per sample
    uint32_t SubChunk2ID;   // "data"
    uint32_t SubChunk2Size; // Size of the data chunk
} WAV_FormatTypeDefCustom;

class SampleVoice
{
  private:
    uint32_t bufferLength;
    size_t   length;
    size_t   position;
    bool     playing;
    int16_t *buffer;
    FIL      fp;
    bool     stereo;

  public:
    // buffer should be predefined in SDRAM
    void Init(uint32_t bufferLength)
    {
        this->bufferLength = bufferLength;
        buffer             = new int16_t[bufferLength];
        length             = 0;
        position           = 0;
        playing            = false;
    }

    ~SampleVoice() { delete[] buffer; }

    // starts sample playing. need to add handling of sample already playing.
    void Play() { playing = true; }

    float Stream()
    {
        if(playing)
        {
            if(position >= length)
            {
                playing  = false;
                position = 0;
                return 0.0;
            }
            return s162f(buffer[position++]);
        }
        return 0.0;
    }

    void SetLength(size_t length) { this->length = length; }

    void *GetBuffer() { return (void *)buffer; }

    uint32_t GetBufferLength() { return bufferLength; }

    bool IsStereo() { return stereo; }
    bool IsMono() { return !stereo; }

    /* adds given file to the buffer. Only supports 16bit PCM 48kHz. 
     * If Stereo samples are interleaved left then right.
     * return 0: succesful, 1: file read failed, 2: invalid format, 3: file too large
    */
    int SetSample(const char *fname)
    {
        UINT                    bytesread;
        WAV_FormatTypeDefCustom wav_data;

        memset(buffer, 0, bufferLength);

        if(f_open(&fp, fname, (FA_OPEN_EXISTING | FA_READ)) == FR_OK)
        {
            // Populate the WAV Info
            if(f_read(&fp,
                      (void *)&wav_data,
                      sizeof(WAV_FormatTypeDefCustom),
                      &bytesread)
               != FR_OK)
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }

        if(wav_data.SampleRate != 48000 || wav_data.BitPerSample != 16)
        {
            return 2;
        }
        if(wav_data.SubChunk2Size > bufferLength || wav_data.NbrChannels > 2)
        {
            return 3;
        }
        stereo = wav_data.NbrChannels == 2;

        if(f_lseek(&fp, sizeof(WAV_FormatTypeDefCustom)) != FR_OK)
        {
            return 1;
        }
        if(f_read(&fp, buffer, wav_data.SubChunk2Size, &bytesread) != FR_OK)
        {
            return 1;
        }
        length = bytesread / (wav_data.BitPerSample / 8);

        f_close(&fp);
        return 0;
    }
};
