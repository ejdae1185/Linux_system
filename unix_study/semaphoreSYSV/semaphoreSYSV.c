#include "semaphoreSYSV.h"

void semhandle(key_t key)
{
    int pid = getpid();
    int semid = initSem(key);
    semLock(semid);
    // test(semid);
    printf("Lock : Process %d\n", pid);
    int sleep_time = 2;
    while (sleep_time > 0)
    {
        printf("Remaining seconds: %d\n", sleep_time);
        sleep(1);
        sleep_time--;
    }
    test(semid);
    semULock(semid);
    printf("Unlock : Process %d\n", pid);
}

int main()
{
    key_t key = 12345;
    for (int a = 0; a < 3; a++)
    {
        if (fork() == 0)
        {
            printf("Child process %d\n", getpid());
            semhandle(key);
        }
    }
    wait(NULL);
    return 0;
}