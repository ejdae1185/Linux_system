// header for Linux sound control and gui by glfw

#ifndef TYPE_H
#define TYPE_H
#define SIGUSR3 30
// #include "messageModule.h"
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <GLFW/glfw3.h>
#include <stdbool.h> // Add this line to include the 'stdbool.h' header file
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <time.h>
#include <mqueue.h>    // mqd_t와 POSIX 메시지 큐 관련 함수들을 포함
#include <errno.h>     // errno 변수를 사용하기 위해
#include <sys/stat.h>  // 파일의 속성을 변경하기 위해
#include <sys/types.h> // POSIX 메시지 큐를 사용하기 위해
#include <signal.h>    // 시그널 처리를 위해

#define FIFO "myfifo"
#define SHARED_MEMORY_KEY 12344

// define
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

// enum
typedef enum CMD
{
    CMD_DEFAULT,
    // sound module
    CMD_SOUND_MODULE_RECODE_START,
    CMD_SOUND_MODULE_RECODE_STOP,
    // draw module
    // message module
    CMD_MSSAFGE_MOCDULE_SEND,
    CMD_MESSAGE_MOCDULE_RECEIVE,

    CMD_MODDULE_CREATE,
    CMD_MODDULE_DESTROY,
    CMD_MODDULE_DESTROY_DOEN,
    CMD_MODDULE_CREATE_DONE,
} eCMD;

typedef enum Modules
{
    MODULE_DEFAULT,
    MODULE_MAIN,
    MODULE_DRAW,
    MODULE_SOUND,
    MODULE_MESSAGE,
} eModule;

typedef enum _module_mode_status
{
    MODE_DEFAULT,
    MODULE_MODE_OFF,
    MODULE_MODE_READY,
    MODULE_MODE_ON,
    MODULE_MODE_FAIL,
} eModuleModeStat_t;

// data structure

typedef struct _module_info
{
    pthread_t thread;
    bool isDefault;
    void *(*function)(void *);
    eModuleModeStat_t status;
    char name[32];
    struct timespec timestamp;

} ModuleInfo_t;

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

typedef struct RecodeInfo
{
    int fd;
    uint32_t channels;
    uint32_t rate;
    uint32_t format;
    uint32_t period_time;
    uint32_t time;
    uint32_t dir;
    char *buffer;
    uint32_t pcm;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    uint32_t allocatedFrames;
    uint32_t data_size;

} RecodeInfo_t;

typedef struct msgBuf
{
    eModule to;
    eModule from;
    eCMD cmd;
    char data[256];
} Message;

#endif // TYPE_H