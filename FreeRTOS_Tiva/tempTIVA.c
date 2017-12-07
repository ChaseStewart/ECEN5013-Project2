/****************************************************************
* file  : tempTIVA.c
* brief : This file implements functions related to handling
*         temperature sensor on TIVA using FreeTOS
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#include "common.h"
#include "tempTIVA.h"

/*Global variable for Queue Handles in main*/
extern QueueHandle_t mainQueue;
extern QueueHandle_t tempQueue;
extern QueueHandle_t lightQueue;
extern QueueHandle_t socketQueue;
extern QueueHandle_t soilQueue;
extern QueueHandle_t chargeQueue;

extern bool stateRunning;

void tempTask(void *pvParameters)
{
    message_t queueData;                /*Variable to store msgs read from queue*/
    uint32_t notificationValue = 0;
    while(stateRunning)
    {
        xTaskNotifyWait(0x00, ULONG_MAX, &notificationValue, portMAX_DELAY);   /*Blocks indefinitely waiting for notification*/
        if(notificationValue & TASK_NOTIFYVAL_HEARTBEAT)
        {
           sendHeartBeat(TEMP_TASK_ID);
        }
        if(notificationValue & TASK_NOTIFYVAL_MSGQUEUE)
        {
            while(errQUEUE_EMPTY != xQueueReceive(tempQueue,(void*)&queueData,0))                     /*Non-blocking call, Read Until Queue is empty*/
            {
                UARTprintf("\r\nTemp Task Received a Queue Data");
                if(queueData.id == HEARTBEAT_REQ)
                {
                    sendHeartBeat(TEMP_TASK_ID);
                }
                if(queueData.id == TEMP_DATA_REQ)
                {

                }
            }
        }
    }
    vTaskDelete(NULL);  /*Deletes Current task and frees up memory*/
}
