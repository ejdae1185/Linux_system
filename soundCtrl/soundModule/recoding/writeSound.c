#include "writeSound.h"

bool isRecording = false;  // 전역 변수로 선언
#define BITS_PER_SAMPLE 16 // 일반적인 PCM 파일의 비트 깊이

void setWavHeader(RecodeInfo_t *recodeInfo, FILE *file)
{
    WAVHeader header;
    uint32_t data_size = recodeInfo->allocatedFrames * recodeInfo->channels * BITS_PER_SAMPLE / 8;

    memcpy(header.riff, "RIFF", 4);
    header.overall_size = data_size + sizeof(WAVHeader) - 8;
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt_chunk_marker, "fmt ", 4);
    header.length_of_fmt = 16;
    header.format_type = 1; // PCM
    header.channels = recodeInfo->channels;
    header.sample_rate = recodeInfo->rate;
    header.byterate = recodeInfo->rate * recodeInfo->channels * BITS_PER_SAMPLE / 8;
    header.block_align = recodeInfo->channels * BITS_PER_SAMPLE / 8;
    header.bits_per_sample = BITS_PER_SAMPLE;
    memcpy(header.data_chunk_header, "data", 4);
    header.data_size = data_size;

    fwrite(&header, sizeof(WAVHeader), 1, file);
}

void rawToWav(RecodeInfo_t *recodeInfo)
{
    // 현재 시간 가져오기
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[100] = {0};
    strftime(filename, sizeof(filename) - 1, "wav_%Y%m%d_%H%M%S.wav", t);
    printf("Generated filename: %s\n", filename);

    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    setWavHeader(recodeInfo, file);
    uint32_t buffer_size = recodeInfo->allocatedFrames * recodeInfo->channels * BITS_PER_SAMPLE / 8;

    fwrite(recodeInfo->buffer, sizeof(char), buffer_size, file);

    printf("Recording saved to %s\n", filename);
    fclose(file);
}
void soundModulation(const char *input_file, const char *output_file, float volume_db, float pitch_cents)
{
    sox_format_t *in, *out;
    sox_effects_chain_t *chain;
    sox_effect_t *effect;
    char *args[10];

    // SoX 라이브러리 초기화
    sox_init();

    // 입력 파일 열기
    in = sox_open_read(input_file, NULL, NULL, NULL);
    if (in == NULL)
    {
        fprintf(stderr, "Failed to open input file\n");
        sox_quit();
        return;
    }

    // 출력 파일 열기
    out = sox_open_write(output_file, &in->signal, &in->encoding, NULL, NULL, NULL);
    if (out == NULL)
    {
        fprintf(stderr, "Failed to open output file\n");
        sox_close(in);
        sox_quit();
        return;
    }

    // 이펙트 체인 생성
    chain = sox_create_effects_chain(&in->encoding, &out->encoding);

    // 입력 이펙트 추가 (필수)
    effect = sox_create_effect(sox_find_effect("input"));
    args[0] = (char *)in;
    sox_effect_options(effect, 1, args);
    sox_add_effect(chain, effect, &in->signal, &in->signal);
    free(effect);

    // 볼륨 변경 이펙트 추가
    effect = sox_create_effect(sox_find_effect("vol"));
    sprintf(args[0], "%.2fdB", volume_db); // 볼륨을 설정 (예: 3dB 증가)
    sox_effect_options(effect, 1, args);
    sox_add_effect(chain, effect, &in->signal, &in->signal);
    free(effect);

    // 피치 변경 이펙트 추가
    effect = sox_create_effect(sox_find_effect("pitch"));
    sprintf(args[0], "%.2f", pitch_cents); // 피치를 설정 (예: 300 cents 증가)
    sox_effect_options(effect, 1, args);
    sox_add_effect(chain, effect, &in->signal, &in->signal);
    free(effect);

    // 출력 이펙트 추가 (필수)
    effect = sox_create_effect(sox_find_effect("output"));
    args[0] = (char *)out;
    sox_effect_options(effect, 1, args);
    sox_add_effect(chain, effect, &in->signal, &in->signal);
    free(effect);

    // 이펙트 체인 실행
    sox_flow_effects(chain, NULL, NULL);

    // 메모리 해제 및 종료
    sox_delete_effects_chain(chain);
    sox_close(in);
    sox_close(out);
    sox_quit();
}

bool writeSound(snd_pcm_t *pcmHandle, RecodeInfo_t *recodingInfo)
{
    uint32_t totalFramesRead = 0;

    while (isRecording)
    {
        printf("Recording...\n");
        int framesRead = snd_pcm_readi(pcmHandle, recodingInfo->buffer + totalFramesRead * 4, recodingInfo->frames);

        if (framesRead == -EPIPE)
        {
            printf("XRUN.\n");
            snd_pcm_prepare(pcmHandle);
        }
        else if (framesRead < 0)
        {
            printf("ERROR: Can't read from PCM device. %s\n", snd_strerror(framesRead));
            return false;
        }
        else if (framesRead != (int)recodingInfo->frames)
        {
            printf("Short read, read %d frames\n", framesRead);
            return false;
        }

        totalFramesRead += framesRead;

        if (totalFramesRead >= recodingInfo->allocatedFrames)
        {
            recodingInfo->allocatedFrames *= 2;
            char *newBuffer = (char *)realloc(recodingInfo->buffer, 4 * recodingInfo->allocatedFrames);
            if (newBuffer == NULL)
            {
                printf("ERROR: Can't allocate memory\n");
                return false;
            }
            recodingInfo->buffer = newBuffer;
        }
    }

    return true;
}

void saveRecordingToFile(RecodeInfo_t *recodeInfo)
{

    // 현재 시간 가져오기
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[100];

    // 파일 이름 생성
    strftime(filename, sizeof(filename) - 1, "recode_%Y%m%d_%H%M%S.raw", t);

    // 파일 열기
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    // 녹음된 데이터 저장
    fwrite(recodeInfo->buffer, 1, recodeInfo->allocatedFrames * 4, file);
    fclose(file);

    printf("Recording saved to %s\n", filename);
    rawToWav(recodeInfo);
}

void stop_writeSound(RecodeInfo_t *recodeInfo)
{
    // runnig time check
    clock_t start = clock();
    isRecording = false;
    saveRecordingToFile(recodeInfo);
    clock_t end = clock();
    printf("Recording time: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
}

void *writeSoundThread(void *arg)
{
    snd_pcm_t *pcmHandle = (snd_pcm_t *)arg;
    RecodeInfo_t recodingInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    writeSound(pcmHandle, &recodingInfo);
    return NULL;
}