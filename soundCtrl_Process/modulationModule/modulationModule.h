#include <type.h>
#include <system.h>
#include <alsa/asoundlib.h>
#include <sox.h>

void soundModulation(const char *input_file, const char *output_file, float volume_db, float pitch_cents);
