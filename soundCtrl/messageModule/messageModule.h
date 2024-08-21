#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>    // O_* 상수
#include <sys/stat.h> // 모드 상수
#include "type.h"

#define MSG_QUEUE_KEY 1234
#define QUEUE_DRAW_MODULE "/draw_queue"
#define QUEUE_SOUND_MODULE "/sound_queue"
#define QUEUE_MAIN "/main_queue"
#define QUEUE_MESSAGE_MODULE "/message_queue"

mqd_t initMessageQueue(const char *queue_name);
uint32_t getMessage(mqd_t mq, Message *rx);
uint32_t sendMessage(mqd_t mq, Message *msg, pthread_mutex_t *mutex);
void routeMessages(eModule to, eModule from, Message *msg);
void *messageModule();

#endif // MESSAGE_QUEUE_H