/****************************************************************
* file  : socketTIVA.h
* brief : This file implements the socket functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef SOCKETTIVA_H_
#define SOCKETTIVA_H_

/****************************************************************
* name  : socketTask
* brief : This function is the start of the socket Tasks which
*         handles sending and receiving information over the socket
* param : Passed when creating the task
* return: None
*****************************************************************/
void socketTask(void *pvParameters);

#endif /* SOCKETTIVA_H_ */
