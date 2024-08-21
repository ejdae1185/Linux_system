#include "outputModule.h"
#include <time.h>

bool revSigUsr3 = false;
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

char *rawToWav(RecodeInfo_t *recodeInfo, char *buffer, char *wavfilePath)
{
    // // 현재 시간 가져오기
    // time_t now = time(NULL);
    // struct tm *t = localtime(&now);
    // strftime(wavfilePath, 100 - 1, "wav_%Y%m%d_%H%M%S.wav", t);
    printf("Generated filename: %s\n", wavfilePath);

    FILE *file = fopen(wavfilePath, "wb");
    if (!file)
    {
        perror("Failed to open file");
        return 1;
    }

    setWavHeader(recodeInfo, file);
    uint32_t buffer_size = recodeInfo->data_size;

    fwrite(buffer, buffer_size, 1, file);

    printf("Recording saved to %s\n", wavfilePath);
    fclose(file);

    return wavfilePath;
}

char *saveRecordingToFile(RecodeInfo_t *recodeInfo, char *buffer, char *rawfilePath, char *wavfilePath)
{

    // 현재 시간 가져오기
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // 파일 이름 생성
    // strftime(rawfilePath, 100 - 1, "recode_%Y%m%d_%H%M%S.raw", t);

    // PrintDbg("filename: %s\n", rawfilePath);

    // // 파일 열기
    FILE *file = fopen(rawfilePath, "wb");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }
    else
    {
        PrintDbg("file opened\n");
    }
    size_t length = strlen(buffer);

    printf("String length: %zu\n", length);
    // recodeINfo->buffer에 데이터가 저장된건지 확

    // 녹음된 데이터 저장

    size_t bytesWritten = fwrite(buffer, 1, recodeInfo->data_size, file);

    printf("Recording saved to %s\n", rawfilePath);
    wavfilePath = rawToWav(recodeInfo, buffer, wavfilePath);
    printf("wawfilename: %s\n", wavfilePath);
    return wavfilePath;
}

void getDataFromSharedMemory(RecodeInfo_t *recodeInfo, char **buffer)
{
    printf("------------------------------\n");
    int sharedMemoryId = shmget(SHARED_MEMORY_KEY, sizeof(RecodeInfo_t), 0666 | IPC_CREAT);
    printf("sharedMemoryId: %d\n", sharedMemoryId);
    void *shmaddr = shmat(sharedMemoryId, NULL, 0);

    *recodeInfo = *(RecodeInfo_t *)shmaddr; // 이중 포인터를 통해 주소를 변경
    printf("allocatedFrames :%d\n", recodeInfo->allocatedFrames);
    printf("size = %d\n", recodeInfo->data_size);
    printf("shmaddr: %p\n", shmaddr);
    printf("recodeInfo: %p\n", recodeInfo);
    *buffer = (char *)malloc(recodeInfo->data_size);
    memcpy(*buffer, shmaddr + sizeof(RecodeInfo_t), recodeInfo->data_size);

    // 메모리 해제
    shmdt(shmaddr);
    shmctl(sharedMemoryId, IPC_RMID, NULL);
    printf("------------------------------\n");
}

pid_t makeProcess(char *path)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        execl(path, path, (char *)NULL);
        perror("exec failed");
        exit(1);
    }
    else if (pid > 0)
    {
        printf("pid = %d\n", pid);
        return pid;
    }
    else
    {
        perror("fork failed");
        exit(1);
    }
}

void linkpipe(char *soundfilePath)
{
    struct stat st;
    if (stat(FIFO, &st) == -1)
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "FIFO does not exist\n");
        }
        else
        {
            perror("stat failed");
        }
    }
    else
    {
        int fp = open(FIFO, O_WRONLY);
        if (fp == -1)
        {
            perror("open failed");
        }
        else
        {
            // FIFO opened successfully, proceed with writing
            printf("path = %s\n", soundfilePath);
            write(fp, soundfilePath, sizeof(soundfilePath));
            // ...
            close(fp);
        }
    }
}

void modulationSigHander(int sig)
{
    revSigUsr3 = true;
}
char rawfilePath[100] = "raw.raw";
char wavfilePath[100] = "wav.wav";
void main()
{
    RecodeInfo_t *recodeInfo = (RecodeInfo_t *)malloc(sizeof(RecodeInfo_t));
    char *buffer;
    char *filename = NULL;
    // time_t now = time(NULL);
    // struct tm *t = localtime(&now);
    // strftime(rawfilePath, sizeof(rawfilePath) - 1, "raw.raw", t);
    // strftime(wavfilePath, sizeof(wavfilePath) - 1, "wav.wav", t);
    // shared memory에서 recodeInfo를 읽어옴
    getDataFromSharedMemory(recodeInfo, &buffer);
    printf("recodeInfo: %p\n", recodeInfo);
    printf("buffer: %p\n", buffer);
    if (recodeInfo != NULL)
    {
        PrintDbg("outputModule: saveRecordingToFile\n");
        filename = saveRecordingToFile(recodeInfo, buffer, rawfilePath, wavfilePath);
        printf("output : filename: %s\n", filename);
    }
    else
    {
        fprintf(stderr, "Failed to retrieve data from shared memory.\n");
    }
    // 메모리 해제
    if (buffer != NULL)
    {
        free(buffer);
    }
    if (recodeInfo != NULL)
    {
        free(recodeInfo);
    }

    struct sigaction sigUsr3;
    sigUsr3.sa_handler = modulationSigHander;
    sigaction(SIGUSR3, &sigUsr3, NULL);

    while (true)
    {

        if (revSigUsr3)
        {
            PrintDbg("outputModule=> modulation\n");
            printf("filename: %s\n", wavfilePath);
            linkpipe(wavfilePath);
            pid_t modulationPid = makeProcess("../../modulationModule/build/modulationModule");
            printf("send to modulationPid = %d\n", modulationPid);

            break;
        }
    }
}