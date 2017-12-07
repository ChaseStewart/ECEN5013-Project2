/****************************************************************
* file  : common.c
* brief : This file implements the macros, functions and structures
*         which are used in common across the project
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 12/06/2017
*****************************************************************/

#include "common.h"

extern QueueHandle_t mainQueue;
extern TaskHandle_t mainTaskHandle;

int8_t sendHeartBeat(Task_Id taskId)
{
    message_t hbMessage;
    hbMessage.id = HEARTBEAT_RSP;
    hbMessage.source = taskId;
    hbMessage.timestamp = 0;
    hbMessage.data.message = 0;     /*Logging will send string pointer to socket Task which inturn will store logs on BBG*/
    hbMessage.length = 0;
    if(pdPASS != xQueueSend(mainQueue,(void*)&hbMessage,portMAX_DELAY))    /*HeartBeat is high priority, so it can block if queue is full*/
    {
        UARTprintf("\r\nHeartBeat sending failed");
        return -1;
    }
    xTaskNotify(mainTaskHandle,TASK_NOTIFYVAL_MSGQUEUE, eSetBits);         /*Always will return pdPASS in this use-case*/
    return 0;
}
