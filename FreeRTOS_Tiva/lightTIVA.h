/****************************************************************
* file  : lightTIVA.h
* brief : This file implements functions related to handling
*         light sensor on TIVA using FreeTOS
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef LIGHTTIVA_H_
#define LIGHTTIVA_H_

/****************************************************************
* name  : lightTask
* brief : This function is the start of the Light Tasks which
*         handles Light sensor
* param : Passed when creating the task
* return: None
*****************************************************************/
void lightTask(void *pvParameters);

#endif /* LIGHTTIVA_H_ */
