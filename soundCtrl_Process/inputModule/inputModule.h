#include <type.h>
#include <system.h>
#include <alsa/asoundlib.h>
#include <sox.h>
#define SAMPLE_RATE 44100
#define CHANNELS 2
#define RECORDING_TIME 5
#define PCM_DEVICE "default" // 기본 PCM 장치로 설정
bool writeSound(snd_pcm_t *pcmHandle, RecodeInfo_t *recodingInfo);
void releaseToSharedMemory(RecodeInfo_t *rearecodingInfo);
int _checkPage(size_t size);
void recodeHandler(int sig);
void initRecodeInfo(RecodeInfo_t *recodeInfo);
snd_pcm_t *openPcmDevice(RecodeInfo_t *recodeInfo, const char *deviceName);