/****************************************************************
* file  : main.h
* brief : This file implements main task functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef MAIN_H_
#define MAIN_H_

#include "common.h"

/*System clock rate, 120 MHz*/
#define SYSTEM_CLOCK    120000000U

/*Function Declarations*/

/*****************************************************************
* name  : sendDataFromMain
* brief : This function is the start of the Temperature Tasks which
*         handles temperature
* param : QueueHandle_t - Queue Handle, int32_t - Integer data to be sent
* return: 0 on SUCCESS, -1 on Failure
*****************************************************************/
int8_t sendDataFromMain(QueueHandle_t queue, Message_Type msgID, int32_t data);

/*****************************************************************
* name  : logFromMain
* brief : This function is the start of the Temperature Tasks which
*         handles temperature
* param : uint8_t* - Pointer to the string
* return: 0 on SUCCESS, -1 on Failure
*****************************************************************/
int8_t logFromMain(uint8_t* data);


#endif /* MAIN_H_ */
