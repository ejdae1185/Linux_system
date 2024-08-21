#include <type.h>
#include <system.h>
#include <alsa/asoundlib.h>
#include <sox.h>

char *saveRecordingToFile(RecodeInfo_t *recodeInfo, char *buffer, char *rawfilePath, char *wavfilePath);
char *rawToWav(RecodeInfo_t *recodeInfo, char *buffer, char *wavfilePath);
void setWavHeader(RecodeInfo_t *recodeInfo, FILE *file);
