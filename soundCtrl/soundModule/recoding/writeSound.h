#ifndef WRITE_SOUND_H
#define WRITE_SOUND_H

#include <type.h>
#include <system.h>
#include <alsa/asoundlib.h>
#include <sox.h>

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define RECORDING_TIME 5

typedef struct WAVHeader
{
    char riff[4];              // "RIFF"
    uint32_t overall_size;     // 파일 전체 크기 - 8 바이트
    char wave[4];              // "WAVE"
    char fmt_chunk_marker[4];  // "fmt "
    uint32_t length_of_fmt;    // fmt chunk의 크기 (16 bytes for PCM)
    uint16_t format_type;      // PCM = 1 (linear quantization)
    uint16_t channels;         // 채널 수
    uint32_t sample_rate;      // 샘플 레이트 (blocks per second)
    uint32_t byterate;         // SampleRate * NumChannels * BitsPerSample/8
    uint16_t block_align;      // NumChannels * BitsPerSample/8
    uint16_t bits_per_sample;  // 비트 깊이
    char data_chunk_header[4]; // "data"
    uint32_t data_size;        // 샘플 데이터의 크기
} WAVHeader;

bool writeSound(snd_pcm_t *pcmHandle, RecodeInfo_t *recodingInfo);
void stop_writeSound(RecodeInfo_t *recodeInfo);

void stopRecording();
void *writeSoundThread(void *arg);

#endif // WRITE_SOUND_H