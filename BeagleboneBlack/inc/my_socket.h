

#include <mysql/mysql.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <mqueue.h>

#ifndef __MY_SOCKET_H__
#define __MY_SOCKET_H__



void * mainSocket(void *arg);

int8_t logFromSocket(mqd_t queue, int prio, char *message);

int8_t initSocketQueues(mqd_t *main_queue, mqd_t *logger_queue, mqd_t *socket_queue);

int8_t initSocket(int *sock_handle);

int8_t sendMessage(message_t *in_message);

#endif
