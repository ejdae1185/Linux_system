#include "modulationModule.h"
char recvfilename[100] = {0};
void soundModulation(const char *input_file, const char *output_file, float volume_db, float pitch_cents)
{
    sox_format_t *in, *out;
    sox_effects_chain_t *chain;
    sox_effect_t *effect;
    char *args[10];

    // SoX 라이브러리 초기화
    sox_init();

    // 입력 파일 열기
    printf("Input file: %s\n", input_file);
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
    printf("volume_db: %f\n", volume_db);
    sprintf(args[0], "%.2fdB", volume_db); // 볼륨을 설정 (예: 3dB 증가)
    sox_effect_options(effect, 1, args);
    sox_add_effect(chain, effect, &in->signal, &in->signal);
    free(effect);

    // 피치 변경 이펙트 추가
    effect = sox_create_effect(sox_find_effect("pitch"));
    printf("pitch_cents: %f\n", pitch_cents);
    sprintf(args[0], "%.2f", pitch_cents); // 피치를 설정 (예: 300 cents 증가)
    sox_effect_options(effect, 1, args);
    sox_add_effect(chain, effect, &in->signal, &in->signal);
    free(effect);

    // 시간 보정 이펙트 추가
    effect = sox_create_effect(sox_find_effect("tempo"));
    char *tempo_args[] = {"-q", "1.0"}; // 시간 변화 없음 (속도 유지)
    sox_effect_options(effect, 2, tempo_args);
    sox_add_effect(chain, effect, &in->signal, &in->signal);
    free(effect);

    // 출력 이펙트 추가 (필수)
    effect = sox_create_effect(sox_find_effect("output"));
    args[0] = (char *)out;
    sox_effect_options(effect, 1, args);
    sox_add_effect(chain, effect, &in->signal, &in->signal);
    free(effect);

    // 노멀라이제이션 이펙트 추가 (마지막 단계)
    effect = sox_create_effect(sox_find_effect("norm"));
    sox_effect_options(effect, 0, NULL);
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

char *readParmByPipe()
{
    int fp = open(FIFO, O_RDONLY | O_NONBLOCK);
    if (fp == -1)
    {
        perror("open failed");
    }
    else
    {
        printf("FIFO opened\n");
    }
    read(fp, recvfilename, sizeof(recvfilename));
    return recvfilename;
}

int main()
{
    PrintDbg("modulationModule: modulation\n");
    readParmByPipe();

    soundModulation(recvfilename, "modulated.wav", 3.0, 1200.0);
}