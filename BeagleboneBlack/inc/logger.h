/*****************************************************
 * Redistribution, modification or use of this software in source or binary forms
 * is permitted as long as the files maintain this copyright. Users are permitted to
 * modify this and use it to learn about the field of embedded software but don't copy
 * my (Chase E Stewart's) work for class, I worked really hard on this. Alex Fosdick and
 * the University of Colorado and Chase E Stewart are not liable for any misuse of this material.
 * License copyright (C) 2017 originally from Alex Fosdick, code by Chase E Stewart.
 *****************************************************/
/*
 * @file logger.h
 * @brief header for the logger file 
 *
 * @author Chase E Stewart and Raj Subramaniam
 * @date December 15 2017
 * @version 1.0
 *
 */

#include <mqueue.h>
#include <time.h>

#include "common.h"


#ifndef __MY_LOGGER_H__
#define __MY_LOGGER_H__

/* logger args */
#define LOGGER_QUEUE_NAME "/logger\x00"
#define LOGGER_MSG_SIZE   100
#define MAIN_NUM_MSGS  	  1000

/* main program for logger */
void *mainLogger(void *);

/* set up the logger to run*/
int8_t initLoggerQueues(mqd_t *main_queue, mqd_t *logger_queue);

/*Function to print the logger structure onto the log file*/
int8_t logMessage(message_t *in_message);

/* 
 * put a message into the logfile from the logger
 * NOTICE queue is unused + only for compatibility here
 */
int8_t logFromLogger(mqd_t queue, int prio, char *message);

#endif
