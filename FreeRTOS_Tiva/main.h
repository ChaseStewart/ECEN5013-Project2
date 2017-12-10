/****************************************************************
* file  : main.h
* brief : This file implements main task functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef MAIN_H_
#define MAIN_H_

#include "common.h"
#include "timers.h"

/*Function Declarations*/

/*****************************************************************
* name  : main
* brief : The top level function for the whole application
* param : None
* return: 0 on SUCCESS, -1 on Failure
*****************************************************************/
int main(void);

/*****************************************************************
* name  : mainTask
* brief : This function is the thread for the main task to handle
*         heartbeats
* param : Pointer to a required data
* return: 0 on SUCCESS, -1 on Failure
*****************************************************************/
void mainTask(void *pvParameters);

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

/*****************************************************************
* name  : hbTimerCB
* brief : Notifies the tasks to send heartbeat when the timer expires
* param : Timer Handle
* return: None
*****************************************************************/
void hbTimerCB(TimerHandle_t xTimer);

/*****************************************************************
* name  : wdTimerCB
* brief : Notifies the main task to check the received heart-beats
* param : Timer Handle
* return: None
*****************************************************************/
void wdTimerCB(TimerHandle_t xTimer);


/****************************************************************
* name  : ADCInit
* brief : Initialize the ADC for the soil sensor
* param : None
* return: None
*****************************************************************/
void myADCInit(void);

/****************************************************************
* name  : I2CInit
* brief : Initialize the I2C bus for fuel gauge, temp sensor, and light sensor
* param : None
* return: None
*****************************************************************/
void myI2CInit(void);



#endif /* MAIN_H_ */
