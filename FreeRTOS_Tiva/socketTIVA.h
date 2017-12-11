/****************************************************************
* file  : socketTIVA.h
* brief : This file implements the socket functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef SOCKETTIVA_H_
#define SOCKETTIVA_H_

#include "utils/lwiplib.h"

/* interrupt priority modes */
#define SYSTICK_INT_PRIORITY        0x80
#define ETHERNET_INT_PRIORITY       0xC0
#define SOCKET_TCP_PRIO             TCP_PRIO_MIN

/* socket definitions */
#define HOST_IP_1                 192
#define HOST_IP_2                 168
#define HOST_IP_3                 141
#define HOST_IP_4                 182



#define SOCKET_IP_1                 192
#define SOCKET_IP_2                 168
#define SOCKET_IP_3                 141
#define SOCKET_IP_4                 110


#define SOCKET_PORT                 50021
#define SOCKET_POLL_INTERVAL        4


/****************************************************************
* name  : socketTask
* brief : This function is the start of the socket Tasks which
*         handles sending and receiving information over the socket
* param : Passed when creating the task
* return: None
*****************************************************************/
void socketTask(void *pvParameters);


/****************************************************************
* name  : socket_accept
* brief : Upon accepting the socket connection, set up callbacks
* param : void *arg - unused
* param : struct tcp_pcb *pcb - the PCB of the TCP socket that received
* param : err_t err- shows whether there was an err with the receive
* return: 0 if success, else err_t
*****************************************************************/
static err_t socket_accept(void *arg, struct tcp_pcb *pcb, err_t err);


/****************************************************************
* name  : socket_recv
* brief : This function is the start of the socket Tasks which
*         handles sending and receiving information over the socket
* param : void *arg - unused
* param : struct tcp_pcb *pcb - the PCB of the TCP socket that received
* param : struct pbuf *p a buffer that holds the received data
* param : err_t err- shows whether there was an err with the receive
* return: 0 if success, else err_t
*****************************************************************/
static err_t socket_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);


/****************************************************************
* name  : socketInit
* brief : This function is used to initialize the ethernet connection
* param : None
* return: None
*****************************************************************/
void tcp_socket_init(uint32_t local_addr);


/****************************************************************
* name  : socket_poll
* brief : This function is used to wait for a status change mostly for closing conn
* param : void *arg - unused
* param : struct tcp_pcb *pcb - the PCB of the TCP socket to poll
* return: 0 if success, else err_t
*****************************************************************/
static err_t socket_poll(void *arg, struct tcp_pcb *pcb);


/****************************************************************
* name  : socket_close_conn
* brief : This function will be used to properly close the socket
* param : struct tcp_pcb *pcb - the PCB of the TCP socket to close
* return: 0 if success, else err_t
*****************************************************************/
static err_t socket_close_conn(struct tcp_pcb *pcb);


/****************************************************************
* name  : socket_sent
* brief : This is called after data is sent via tcpsend, doesn't do much
* param : void *arg           - unused arg
* param : struct tcp_pcb *pcb - the PCB of the TCP socket that just sent
* param : u16_t len           - the len of the sent data
* return: 0 if success, else err_t
*****************************************************************/
static err_t socket_sent(void *arg, struct tcp_pcb *pcb, uint16_t len);


/****************************************************************
* name  : socket_err
* brief : Run this when an error occurs, print it
* param : void *arg - an unused argument
* param : err_t err - the error that had recently occurred.
* return: None
*****************************************************************/
static void socket_err(void *arg, err_t err);


/****************************************************************
* name  : DisplayIPAddress
* brief : display an LWIP style IP address
* param : uint32_t ui32Addr - the int value of the IP address
* return: None
*****************************************************************/
void DisplayIPAddress(uint32_t ui32Addr);


//*****************************************************************************
//
// Required by lwIP library to support any host-related timer functions.
//
//*****************************************************************************
void lwIPHostTimerHandler(void);


void mySysTickHandler(void);


#endif /* SOCKETTIVA_H_ */
