/****************************************************************
* file  : lightTIVA.c
* brief : This file implements functions related to handling
*         light sensor on TIVA using FreeTOS
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#include "common.h"
#include "lightTIVA.h"

/*Global variable for Queue Handles in main*/
extern QueueHandle_t mainQueue;
extern QueueHandle_t tempQueue;
extern QueueHandle_t lightQueue;
extern QueueHandle_t socketQueue;
extern QueueHandle_t soilQueue;
extern QueueHandle_t chargeQueue;

void lightTask(void *pvParameters)
{
    uint32_t queueData;
    for (;;)
    {
        xQueueReceive(lightQueue,&queueData,portMAX_DELAY);       /*This is a test - Fix me*/
        UARTprintf("\r\nLight Received a Queue Data", queueData);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
