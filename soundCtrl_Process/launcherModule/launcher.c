#include "launcher.h"

int main()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        // 에러 처리
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        // 자식 프로세스: 새로운 실행 파일을 실행
        execl("../../viewerModule/build/ViewerModule", "ViewerModule", "arg1", "arg2", (char *)NULL); // exec 호출 후에 이 코드는 실행되지 않음. 실행 실패 시 에러 처리
        perror("exec failed");
        exit(1);
    }
    else
    {
        // 부모 프로세스는 독립적으로 동작
        printf("sound effector launch\n");
        // 부모 프로세스에서 자식의 종료를 기다리려면 다음 코드 추가
        // wait(NULL);
    }

    return 0;
}