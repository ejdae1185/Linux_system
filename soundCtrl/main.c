#include <stdbool.h> // Include the header file that defines the "bool" type
#include <type.h>
#include "system.h"
#include <drawModule.h>
#include <writeSound.h>
#include <soundModule.h>
#include <messageModule.h>

uint32_t mainQueueId;
/* Define macros and structures */
#define MODULE_ADD(def, func, num) \
    {                              \
        .thread = 0,               \
        .isDefault = def,          \
        .function = func,          \
        .name = #func "_" #num,    \
    }

static void mSleep(int milliseconds)
{
    usleep(milliseconds * 1000);
}
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static void __attribute__((optimize("O0"))) waitTillInitIntf(ModuleInfo_t *mod)
{
    while (mod->status != MODULE_MODE_READY)
    {
        mSleep(20);
    }
}

static void create_thread(ModuleInfo_t *mod)
{
    if (!mod)
    {
        printf("Module is NULL\n");
        return;
    }
    printf("Try to turn on Module: %s\n", mod->name);
    int c = pthread_create(&mod->thread, NULL, mod->function, NULL);
    if (c != 0)
    {
        printf("Thread creation failed\n");
        return;
    }
    else
    {
        printf("Thread creation success\n");
    }
    // waitTillInitIntf(mod);

    mod->status = MODULE_MODE_READY;
}

void delocateModule(ModuleInfo_t *mod)
{
    if (mod->status == MODULE_MODE_ON)
    {
        mod->status = MODULE_MODE_OFF;
        pthread_join(mod->thread, NULL);
        printf("Module %s is deleted\n", mod->name);
    }
}

ModuleInfo_t modules[] = {
    MODULE_ADD(true, messageModule, 1),
    MODULE_ADD(true, drawModule, 2),
    MODULE_ADD(true, soundModule, 3)};

int main(void)
{
    mainQueueId = initMessageQueue(QUEUE_MAIN);
    Message *rx = malloc(sizeof(Message));

    // module create
    for (size_t i = 0; i < ARRAY_SIZE(modules); i++)
    {
        create_thread(&modules[i]);
        PrintDbg("Module %s is created", modules[i].name);
        mSleep(100);
    }

    while (1)
    {
        getMessage(mainQueueId, rx);

        switch (rx->cmd)
        {
        case CMD_MODDULE_CREATE_DONE:
            printf("%s module is created\n", rx->data);
            break;

        default:
            break;
        }
    }

    printf("Main function has exited successfully.\n");

    return 0;
}