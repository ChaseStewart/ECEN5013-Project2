/****************************************************************
* file  : tempTIVA.h
* brief : This file implements functions related to handling
*         temperature sensor on TIVA using FreeTOS
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef TEMPTIVA_H_
#define TEMPTIVA_H_

/****************************************************************
* name  : tempTask
* brief : This function is the start of the Temperature Tasks which
*         handles temperature
* param : Passed when creating the task
* return: None
*****************************************************************/
void tempTask(void *pvParameters);

#endif /* TEMPTIVA_H_ */
