#include <sys/socket.h>
#include <arpa/inet.h>
#include <mqueue.h>

#ifndef __MY_SOCKET_H__
#define __MY_SOCKET_H__

#define BBG_SERVER_HOST "127.0.0.1"
#define BBG_SERVER_PORT 50010
#define MAX_NUM_CONNS   5

#define WEBSERVER_HOST "127.0.0.1"
#define WEBSERVER_PORT 80
#define MAX_NUM_CONNS   5


/* struct for packets */
typedef struct
{	
	Message_Type id;	/*Type of information from the Client*/
	size_t dataLen;		/*Length of the data*/
	uint8_t *data;		/*Actual data in the packet*/
}packet_t;


void * mainSocket(void *arg);

int8_t logFromSocket(mqd_t queue, int prio, char *message);

int8_t initSocketQueues(mqd_t *main_queue, mqd_t *logger_queue, mqd_t *socket_queue);

int8_t initSocket(int *sock_handle);

int8_t sendMessage(message_t *in_message);

#endif
