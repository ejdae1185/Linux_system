#include "soundModule.h"
#include "messageModule.h"
pthread_t record_thread;
pthread_t record_stop_thread;
#define PCM_DEVICE "default" // 기본 PCM 장치로 설정
#define SAMPLE_RATE 44100    // 샘플링 레이트 (44.1 kHz)
#define CHANNELS 2           // 스테레오
#define BUFFER_SIZE 2048     // 버퍼 크기
bool recording = false;
mqd_t soundQueueId;
Message *rx = NULL;
Message *tx = NULL;

RecodeInfo_t recodeInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void initRecodeInfo(RecodeInfo_t *recodeInfo)
{
    recodeInfo->fd = -1;
    recodeInfo->channels = 2;     // 스테레오
    recodeInfo->rate = 44100 / 2; // 샘플링 레이트 (44.1 kHz)
    recodeInfo->format = SND_PCM_FORMAT_S16_LE;
    recodeInfo->period_time = 0;
    recodeInfo->time = 0;
    recodeInfo->dir = 0;
    recodeInfo->pcm = 0;
    recodeInfo->handle = NULL;
    recodeInfo->params = NULL;
    recodeInfo->frames = 32;
    recodeInfo->allocatedFrames = 1024;
    recodeInfo->buffer = (char *)malloc(4 * recodeInfo->allocatedFrames);
}

snd_pcm_t *openPcmDevice(RecodeInfo_t *recodeInfo, const char *deviceName)
{
    initRecodeInfo(recodeInfo);
    deviceName = PCM_DEVICE;
    snd_pcm_hw_params_t *params;
    int err;

    err = snd_pcm_open(&recodeInfo->handle, deviceName, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0)
    {
        fprintf(stderr, "Unable to open PCM device: %s\n", snd_strerror(err));
        return NULL;
    }

    // PCM 장치 설정
    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(recodeInfo->handle, params);
    snd_pcm_hw_params_set_access(recodeInfo->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(recodeInfo->handle, params, recodeInfo->format);
    snd_pcm_hw_params_set_channels(recodeInfo->handle, params, recodeInfo->channels);

    // 샘플링 레이트 설정
    unsigned int rate = recodeInfo->rate;
    err = snd_pcm_hw_params_set_rate_near(recodeInfo->handle, params, &rate, 0);
    if (err < 0)
    {
        fprintf(stderr, "Unable to set rate: %s\n", snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return NULL;
    }

    // 주기 크기 설정
    snd_pcm_uframes_t frames = recodeInfo->frames;
    err = snd_pcm_hw_params_set_period_size_near(recodeInfo->handle, params, &frames, 0);
    if (err < 0)
    {
        fprintf(stderr, "Unable to set period size: %s\n", snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return NULL;
    }

    err = snd_pcm_hw_params(recodeInfo->handle, params);
    if (err < 0)
    {
        fprintf(stderr, "Unable to set HW parameters: %s\n", snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return NULL;
    }

    snd_pcm_hw_params_free(params);

    // 실제 설정된 값 확인
    snd_pcm_hw_params_current(recodeInfo->handle, params);
    snd_pcm_hw_params_get_rate(params, &rate, 0);
    snd_pcm_hw_params_get_period_size(params, &frames, 0);
    printf("Rate: %u, Period size: %lu\n", rate, frames);

    recodeInfo->params = params;
    recodeInfo->frames = frames;

    return recodeInfo->handle;
}

void *soundModule()
{
    snd_pcm_t *pcmHandle = openPcmDevice(&recodeInfo, PCM_DEVICE);
    soundQueueId = initMessageQueue(QUEUE_SOUND_MODULE);
    rx = malloc(sizeof(Message));
    tx = malloc(sizeof(Message));

    rx->to = MODULE_SOUND;

    while (true)
    {
        if (getMessage(soundQueueId, rx))
        {
            printf("SoundModule received message: %s\n", rx->data);
        }

        switch (rx->cmd)
        {
        case CMD_SOUND_MODULE_RECODE_START:
            if (!recording)
            {
                recording = true;                   // 녹음 시작
                writeSound(pcmHandle, &recodeInfo); // 녹음 시작
            }
            rx->cmd = CMD_DEFAULT;
            break;
        case CMD_SOUND_MODULE_RECODE_STOP:
            if (recording)
            {
                stop_writeSound(&recodeInfo); // 녹음을 중지
                // pthread_join(record_thread, NULL); // 스레드가 종료될 때까지 기다림
                recording = false;
            }
            rx->cmd = CMD_DEFAULT;
            break;

        default:
            rx->cmd = CMD_DEFAULT;
            break;
        }
    }
    return NULL;
}