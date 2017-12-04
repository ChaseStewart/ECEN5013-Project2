#define _POSIX_SOURCE
#define _POSIX_C_SOURCE (200112L)

#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>

#ifndef __MY_COMMON_H__
#define __MY_COMMON_H__

#define NUM_TASKS 2
#define MAX_FILELEN 1024
#define MAX_MSGLEN  2048

/* queue vars */
#define QUEUE_MSG_SIZE          100 
#define QUEUE_NUM_MSGS          2048
#define MAIN_QUEUE_NAME         "/heartbeat\x00"
#define LOGGER_QUEUE_NAME       "/logger\x00"
#define CLIENT_QUEUE_NAME       "/client\x00"
#define SOCKET_QUEUE_NAME       "/mysocket\x00"

/* message state vars */
#define STATE_ERROR   -1
#define STATE_SHUTDOWN 0
#define STATE_STARTUP  1
#define STATE_RUNNING  2
#define STATE_REQ_RSP  3
#define STATE_REQ_ONLY 4

/* signal variables */
#define HEARTBEAT_SIGNO    35
#define LOGGER_SIGNO       36
#define SOCKET_SIGNO       37
#define CLIENT_SIGNO       39

/* max and min vals for sensors */
#define LIGHT_MAX 100
#define LIGHT_MIN 0
#define HUMID_MAX 100
#define HUMID_MIN 0
#define CHARGE_MAX 100
#define CHARGE_MIN 0
#define TEMP_MAX 100
#define TEMP_MIN 0

/* log levels*/
#define LOG_INFO 0
#define LOG_DEBUG 1
#define LOG_ERROR 2
#define LOG_CRITICAL 3

/* state vars for each process */
extern volatile int main_state;
extern volatile int logger_state;
extern volatile int socket_state;


/* define message_type */
typedef enum message_type {	
	HEARTBEAT_REQ = 0, 
	HEARTBEAT_RSP, 
	TEMP_DATA, 
	TEMP_DATA_REQ,
	LIGHT_DATA, 
	LIGHT_DATA_REQ, 
	LOGGER, 
	FILE_CHANGE,
	CLIENT_SHUTDOWN,
	SOIL_MOIST_DATA,
	SOIL_MOIST_DATA_REQ, 
	FUEL_GAUGE_DATA, 
	FUEL_GAUGE_DATA_REQ, 
	NOTIFY_USER, 
	SOCKET,
	ERROR_OCCURED 
} Message_Type;

/* enumerate each task */
typedef enum task_id 
{
	LOGGER_ID = 0, 
	SOCKET_ID, 
	MAIN_ID, 
	CLIENT_ID
}  Task_Id;

/* Structure for message Queues */
typedef struct
{
	Message_Type id;	/*Log ID - will be an enum*/
	uint32_t timestamp;	/*Timestamp*/
	Task_Id  source;	/*Source*/
	size_t length;		/*Size of the message*/
	char* message;		/*Message Payload*/
	size_t priority;
}message_t;

/* Structure for logger arguments */
typedef struct 
{
	size_t length;
	char *filename;
}logger_args;

/* send heartbeat to provided queue */
int8_t sendHeartbeat(mqd_t queue, Task_Id my_id);

/* block signals used for notification/sigwait */
int8_t blockAllSigs(void);


#endif
