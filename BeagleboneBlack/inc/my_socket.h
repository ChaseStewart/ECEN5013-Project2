/*****************************************************
 * Redistribution, modification or use of this software in source or binary forms
 * is permitted as long as the files maintain this copyright. Users are permitted to
 * modify this and use it to learn about the field of embedded software but don't copy
 * my (Chase E Stewart's) work for class, I worked really hard on this. Alex Fosdick and
 * the University of Colorado and Chase E Stewart are not liable for any misuse of this material.
 * License copyright (C) 2017 originally from Alex Fosdick, code by Chase E Stewart.
 *****************************************************/
/*
 * @file my_socket.h
 * @brief header for the socket server 
 *
 * @author Chase E Stewart and Raj Subramaniam
 * @date December 15 2017
 * @version 1.1
 *
 */

#include <mysql/mysql.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <mqueue.h>

#ifndef __MY_SOCKET_H__
#define __MY_SOCKET_H__


/* create the main socket */
void * mainSocket(void *arg);

/* send a log to the logger from the socket */
int8_t logFromSocket(mqd_t queue, int prio, char *message);

/* create the queues for the socket task */ 
int8_t initSocketQueues(mqd_t *main_queue, mqd_t *logger_queue, mqd_t *socket_queue);

#endif
