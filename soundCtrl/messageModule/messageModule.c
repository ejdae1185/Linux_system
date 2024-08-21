#include "messageModule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t mainMutex;
pthread_mutex_t drawMutex;
pthread_mutex_t messageMutex;
pthread_mutex_t soundMutex;

extern mqd_t drawQueueId;
extern mqd_t mainQueueId;
extern mqd_t soundQueueId;

mqd_t centerQueueId;
Message *rx = NULL;

void cleanupMutexes()
{
    pthread_mutex_destroy(&mainMutex);
    pthread_mutex_destroy(&drawMutex);
    pthread_mutex_destroy(&messageMutex);
    pthread_mutex_destroy(&soundMutex);
}

void signalHandler(int sig)
{
    cleanupMutexes();
    exit(sig);
}

static inline mqd_t _checkModuleMsgQ(eModule module)
{
    mqd_t qId;
    if (module == MODULE_DRAW)
    {
        qId = drawQueueId;
    }
    else if (module == MODULE_SOUND)
    {
        qId = soundQueueId;
    }
    else if (module == MODULE_MAIN)
    {
        qId = mainQueueId;
    }
    return qId;
}

static inline pthread_mutex_t *_checkModuleMutex(eModule modele)
{
    switch (modele)
    {
    case MODULE_DRAW:
        return &drawMutex;
    case MODULE_SOUND:
        return &soundMutex;
    case MODULE_MAIN:
        return &mainMutex;
    case MODULE_MESSAGE:
        return &messageMutex;
    default:
        return NULL;
    }
}

mqd_t initMessageQueue(const char *queue_name)
{

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;               // 큐에 저장할 수 있는 최대 메시지 수
    attr.mq_msgsize = sizeof(Message); // 메시지 크기
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(queue_name, O_CREAT | O_RDWR, 0644, &attr);
    if (mq == (mqd_t)-1)
    {
        perror("mq_open failed");
        exit(EXIT_FAILURE);
    }
    return mq;
}

void receiveMessage(mqd_t mq, Message *msg, pthread_mutex_t *mutex)
{
    if (mq_receive(mq, (char *)msg, sizeof(Message), NULL) == -1)
    {
        perror("mq_receive failed");
        pthread_mutex_unlock(mutex); // 뮤텍스 해제
        exit(EXIT_FAILURE);
    }
}

uint32_t sendMessage(mqd_t mq, Message *msg, pthread_mutex_t *mutex)
{
    if (mq_send(mq, (const char *)msg, sizeof(Message), 0) == -1)
    {
        perror("mq_send failed");
        pthread_mutex_unlock(mutex); // 뮤텍스 해제
        exit(EXIT_FAILURE);
        return 0;
    }
    return 1;
}

uint32_t getMessage(mqd_t mq, Message *rx)
{

    struct mq_attr attr;
    if (mq_getattr(mq, &attr) == -1)
    {
        perror("mq_getattr failed");
        exit(EXIT_FAILURE);
    }

    if (attr.mq_curmsgs > 0)
    {
        pthread_mutex_t *mutex = _checkModuleMutex(rx->to);
        pthread_mutex_lock(mutex); // 뮤텍스 잠금

        receiveMessage(mq, rx, mutex);
        printf("Received message: to=%d, from=%d, cmd=%d, data=%s\n", rx->to, rx->from, rx->cmd, rx->data);
        pthread_mutex_unlock(mutex); // 뮤텍스 해제
        return 1;
    }

    return 0;
}

void routeMessages(eModule to, eModule from, Message *msg)
{
    mqd_t targetQueue = _checkModuleMsgQ(to);
    pthread_mutex_t *targetMutex = _checkModuleMutex(msg->to);

    if (targetMutex != NULL)
    {
        if (pthread_mutex_lock(targetMutex) != 0)
        {
            perror("pthread_mutex_lock failed");
            return;
        }

        sendMessage(targetQueue, msg, targetMutex);

        if (pthread_mutex_unlock(targetMutex) != 0)
        {
            perror("pthread_mutex_unlock failed");
        }

        memset(msg, 0, sizeof(Message));
    }
    else
    {
        fprintf(stderr, "Unknown module: %s\n", to);
    }
}

void _deleteMessageQueue(const char *queueName)
{
    // 큐가 열려있다면 조건 추가
    mqd_t mqd = mq_open(queueName, O_RDONLY);
    if (mqd == (mqd_t)-1)
    {
        perror("Failed to open message queue");
        // Handle the error (e.g., the queue might not exist)
    }
    else
    {
        if (mq_unlink(queueName) == -1)
        {
            perror("Failed to unlink message queue");
        }
        else
        {
            printf("Message queue unlinked successfully\n");
        }
        // Close the queue descriptor
        mq_close(mqd);
    }
}
void *messageModule(void *arg)
{

    // 모든 뮤텍스 초기화
    // cleanupMutexes();
    _deleteMessageQueue(QUEUE_MESSAGE_MODULE);
    _deleteMessageQueue(QUEUE_DRAW_MODULE);
    _deleteMessageQueue(QUEUE_SOUND_MODULE);
    _deleteMessageQueue(QUEUE_MAIN);
    pthread_mutex_init(&mainMutex, NULL);
    pthread_mutex_init(&drawMutex, NULL);
    pthread_mutex_init(&messageMutex, NULL);
    pthread_mutex_init(&soundMutex, NULL);
    centerQueueId = initMessageQueue(QUEUE_MESSAGE_MODULE);
    rx = malloc(sizeof(Message));
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    return NULL;
}