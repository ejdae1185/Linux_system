#include "controlModule.h"

pid_t openPCM()
{
    pid_t pid = fork();
    pid_t inputModulePid = 0;
    // printf("pid = %d\n", pid);

    if (pid == 0)
    {
        execl("../../inputModule/build/InputModule", "InputModule", (char *)NULL);
        perror("exec failed");
        exit(1);
    }
    else if (pid > 0)
    {

        inputModulePid = pid;
        return inputModulePid;
        printf("pid = %d\n", pid);
    }
    else
    {
        perror("fork failed");
        exit(1);
        return inputModulePid;
    }
}

void recode(pid_t inputModulePid)
{
    kill(inputModulePid, SIGUSR1);
}

void recodeStop(pid_t inputModulePid)
{
    kill(inputModulePid, SIGUSR2);
}

pid_t modulation()
{
    pid_t pid = fork();

    if (pid == 0)
    {
        execl("../../modulationModule/build/ModulationModule", "ModulationModule", (char *)NULL);
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