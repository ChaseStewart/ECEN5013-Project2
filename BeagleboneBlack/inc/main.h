#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <getopt.h>

#include "common.h"
#include "logger.h"
#include "my_socket.h"

#ifndef __MY_MAIN_H__
#define __MY_MAIN_H__

/* general vars*/
#define INPUT_LEN 512

#define BBG_SERVER_HOST "192.168.141.110"
#define BBG_SERVER_PORT 50021
#define MAX_NUM_CONNS   5

#define WEBSERVER_HOST "127.0.0.1"
#define WEBSERVER_PORT 80
#define MAX_NUM_CONNS   5

/* commands */
#define ACK_MSG "ACK"
#define NAK_MSG "NAK"
#define DELIM_STR ":\t\n"

#define LED_ON "ON"
#define LED_OFF "OFF"

/* period in secs for heartbeat*/
extern volatile int heartbeat_period;


/* command-line options */
static struct option options[] = {
	{"filename", required_argument, NULL, 'f'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, 0, 0}
};

/*prototypes*/

/* main function for the program */
int main(int argc, char **argv);

/* print a help message*/
void my_print_help(void);

int initThreads(pthread_t *logger_thread, pthread_t *socket_thread);

void handleCtrlC(int sig);

int update_data(int msg_type, int value);

int8_t logFromMain(mqd_t queue, int prio, char *message);

int8_t initMainQueues(mqd_t *main_queue, mqd_t *logger_queue, mqd_t *socket_queue);

int8_t reqHeartbeats(mqd_t logger_queue, mqd_t socket_queue);

int8_t processHeartbeats(mqd_t main_queue, mqd_t logger_queue);

int8_t initMainQueues(mqd_t *main_queue, mqd_t *logger_queue, mqd_t *socket_queue);

#endif
