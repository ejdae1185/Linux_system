#include "inputModule.h"
bool isRecording = true;
bool recodeStoped = false;
RecodeInfo_t recodeInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int sharedMemoryId;
#define sharedMemoryKey 1234
char spinner[] = {'|', '/', '--', '\\'};

pid_t savetoFile()
{
    pid_t pid;
    pid = fork();

    if (pid > 0)
    {
        printf("outputModule pid = %d\n", pid);
        return pid;
    }
    else if (pid == 0)
    {
        execl("../../outputModule/build/outputModule", "outputModule", NULL);
        perror("execl failed");
        exit(1);
    }
    else
    {
        perror("fork failed");
        return -1;
    }
    return 0;
}
void recodeHandler(int sig)
{
    printf("recive  start sig\n");
    isRecording = true;
    writeSound(recodeInfo.handle, &recodeInfo);
}

void recodeStopHandler(int sig)
{
    printf("recive  stop  sig \n");
    isRecording = false;
    // releaseToSharedMemory(&recodeInfo);
}

void exitSigHandler(int sig)
{
    exit(0);
}
int _checkPage(size_t size)
{
    size_t page_size = sysconf(_SC_PAGE_SIZE);
    printf("Page size: %zu bytes\n", page_size);

    if (size % page_size != 0)
    {
        size = ((size / page_size) + 1) * page_size;
        printf("Adjusted size to page boundary: %zu bytes\n", size);
    }
    return size;
}
void releaseToSharedMemory(RecodeInfo_t *recodingInfo)
{
    printf("Releasing data to shared memory\n");

    size_t totalSize = sizeof(RecodeInfo_t) + recodingInfo->data_size;
    totalSize = _checkPage(totalSize);

    sharedMemoryId = shmget(SHARED_MEMORY_KEY, totalSize, 0666 | IPC_CREAT);
    if (sharedMemoryId == -1)
    {
        perror("shmget failed");
        exit(1);
    }
    else
    {
        printf("sharedMemoryId: %d\n", sharedMemoryId);
    }

    void *shmData = shmat(sharedMemoryId, NULL, 0);
    if (shmData == (void *)-1)
    {
        perror("shmat failed");
        exit(1);
    }

    // 구조체 데이터 복사
    memcpy(shmData, recodingInfo, sizeof(RecodeInfo_t));
    // 녹음된 오디오 데이터 복사
    void *bufferLocation = shmData + sizeof(RecodeInfo_t);
    memcpy(bufferLocation, recodingInfo->buffer, recodingInfo->data_size);

    printf("memory address : %p\n", shmData);
    printf("Data released to shared memory\n");

    // 메모리 해제
    shmdt(shmData);

    // 다른 프로세스 시작
    pid_t outputModulePid = savetoFile();
}

bool writeSound(snd_pcm_t *pcmHandle, RecodeInfo_t *recodingInfo)
{
    uint32_t totalFramesRead = 0;
    int spinnerIndex = 0;
    bool start = true;
    recodingInfo->data_size = 0;

    while (isRecording)
    {
        if (start)
        {
            printf("Recording started.\n");
            start = false;
        }
        else
        {
            printf("\r %c", spinner[spinnerIndex]);
            fflush(stdout);
            spinnerIndex = (spinnerIndex + 1) % 4;
        }
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
    printf("------------------------------\n");
    printf("allocatedFrames : %d\n", recodingInfo->allocatedFrames);
    // printf("size = %d\n", recodingInfo->data_size);
    recodingInfo->data_size += totalFramesRead * 4 * sizeof(char);

    printf("size = %d\n", recodingInfo->data_size);

    printf("Recording stopped.\n");
    releaseToSharedMemory(&recodeInfo);

    return true;
}

void initRecodeInfo(RecodeInfo_t *recodeInfo)
{
    recodeInfo->fd = -1;
    recodeInfo->channels = 2;           // 스테레오
    recodeInfo->rate = SAMPLE_RATE / 2; // 22.05kHz
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

void main()
{
    struct sigaction recodeStart;
    struct sigaction recodeStop;
    struct sigaction exit;

    snd_pcm_t *pcmHandle = openPcmDevice(&recodeInfo, PCM_DEVICE);

    recodeStart.sa_handler = recodeHandler;
    sigemptyset(&recodeStart.sa_mask);
    recodeStart.sa_flags = 0;

    recodeStop.sa_handler = recodeStopHandler;
    sigemptyset(&recodeStop.sa_mask);
    recodeStop.sa_flags = 0;

    exit.sa_handler = exitSigHandler;
    sigemptyset(&exit.sa_mask);
    exit.sa_flags = 0;

    // 녹음 시작
    printf("pid = %d\n", getpid());
    signal(SIGUSR1, recodeHandler);

    if (sigaction(SIGUSR1, &recodeStart, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }
    if (sigaction(SIGUSR2, &recodeStop, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    printf("Waiting for signals...\n");
    while (1)
    {
        pause(); // 시그널이 올 때까지 대기
    }
    return NULL;
}