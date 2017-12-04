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

void tempTask(void *pvParameters)
{
    uint32_t queueData = 0;
    while(1)
    {

        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 1);
        vTaskDelay(2000);

        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
        vTaskDelay(2000);

        xQueueSend( lightQueue,&queueData,portMAX_DELAY);  /*This is a test - Fix me*/
        queueData++;
    }
    vTaskDelete(NULL);  /*Deletes Current task and frees up memory*/
}
