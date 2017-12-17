/*****************************************************
 * Redistribution, modification or use of this software in source or binary forms
 * is permitted as long as the files maintain this copyright. Users are permitted to
 * modify this and use it to learn about the field of embedded software but don't copy
 * my (Chase E Stewart's) work for class, I worked really hard on this. Alex Fosdick and
 * the University of Colorado and Chase E Stewart are not liable for any misuse of this material.
 * License copyright (C) 2017 originally from Alex Fosdick, code by Chase E Stewart.
 *****************************************************/
/*
 * @file main.h
 * @brief header for the main server 
 *
 * @author Chase E Stewart and Raj Subramaniam
 * @date December 15 2017
 * @version 1.1
 *
 */


#include <sys/socket.h>
#include <sys/time.h>
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


#define DRIVER_PATH "/dev/myledchar"

#define BBG_SERVER_HOST "192.168.141.182"
#define BBG_SERVER_PORT 50021
#define MAX_NUM_CONNS   5

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

/* shut down the server after receiving a Ctrl+C SIGINT */
void handleCtrlC(int sig);

/* send a log to the logger thread */
int8_t logFromMain(mqd_t queue, int prio, char *message);

/* send a log to the logger thread */
int8_t initMainQueues(mqd_t *main_queue, mqd_t *logger_queue, mqd_t *socket_queue);

/* ask the logger thread and the socket thread for a heartbeat */
int8_t reqHeartbeats(mqd_t logger_queue, mqd_t socket_queue);

/* check for all heartbeats and raise error if not found. */ 
int8_t processHeartbeats(mqd_t main_queue, mqd_t logger_queue);

/* initialize the queues for this thread to use */ 
int8_t initMainQueues(mqd_t *main_queue, mqd_t *logger_queue, mqd_t *socket_queue);

#endif
