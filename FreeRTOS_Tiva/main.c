/****************************************************************
* file  : main.c
* brief : This file implements main task functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/
#include "common.h"
#include "main.h"
#include "lightTIVA.h"
#include "tempTIVA.h"
#include "chargeTIVA.h"
#include "socketTIVA.h"
#include "soilTIVA.h"

/*Global Variables*/
/*Queue Handles*/
QueueHandle_t mainQueue;
QueueHandle_t tempQueue;
QueueHandle_t lightQueue;
QueueHandle_t socketQueue;
QueueHandle_t soilQueue;
QueueHandle_t chargeQueue;

/*Task Handles*/
TaskHandle_t lightTaskHandle;
TaskHandle_t tempTaskHandle;

uint32_t sysClockSet = 0;

/*Main Function*/
int main(void)
{
    // Initialize system clock to 120 MHz
    uint32_t output_clock_rate_hz;
    output_clock_rate_hz = ROM_SysCtlClockFreqSet(
                               (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                               SYSTEM_CLOCK);
    ASSERT(output_clock_rate_hz == SYSTEM_CLOCK);

    sysClockSet = output_clock_rate_hz;

    /*Initialize the GPIO Pins of TIVA*/
    PinoutSet(false, false);

    /*Setup UART connected to Virtual COM Port for logging*/
    UARTStdioConfig(0, 57600, SYSTEM_CLOCK);

    /*Creating Queues*/
    mainQueue = xQueueCreate(MAIN_QUEUE_SIZE, sizeof(message_t));
    if(mainQueue == NULL)
    {
       UARTprintf("\r\nMain Queue Creation Failed");
    }

    tempQueue = xQueueCreate(TEMP_QUEUE_SIZE, sizeof(message_t));
    if(tempQueue == NULL)
    {
        UARTprintf("\r\nTemp Queue Creation Failed");
    }

    lightQueue = xQueueCreate(LIGHT_QUEUE_SIZE, sizeof(message_t));
    if(lightQueue == NULL)
    {
       UARTprintf("\r\nLight Queue Creation Failed");
    }

    socketQueue = xQueueCreate(SOCKET_QUEUE_SIZE, sizeof(message_t));
    if(socketQueue == NULL)
    {
      UARTprintf("\r\nMain Queue Creation Failed");
    }

    /*Create different Tasks*/
    //BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,const char * const pcName,unsigned short usStackDepth,void *pvParameters
    //,UBaseType_t uxPriority,TaskHandle_t *pxCreatedTask );
#if 0
    if(xTaskCreate(tempTask, (const portCHAR *)"TemperatureTask", configMINIMAL_STACK_SIZE, NULL, 1, &tempTaskHandle) != pdPASS)
    {
        UARTprintf("\r\nTemperature Task creation failed");
    }
#endif
    if(xTaskCreate(lightTask, (const portCHAR *)"LightTask", configMINIMAL_STACK_SIZE, NULL, 1, &lightTaskHandle) != pdPASS)
    {
        UARTprintf("\r\nLight Task creation failed");
    }

    /*Random delay given to the main to get synchronize with the new tasks*/
  //  vTaskDelay(pdMS_TO_TICKS(500));

    /*Start the scheduler*/
    vTaskStartScheduler();
   // while(1);

    /*Delete all the queues that were created*/
    vQueueDelete(mainQueue);
    vQueueDelete(tempQueue);
    vQueueDelete(lightQueue);
    vQueueDelete(socketQueue);

    return 0;
}

int8_t sendDataFromMain(QueueHandle_t queue, Message_Type msgID, int32_t data)
{
    message_t logMessage;
    logMessage.id = msgID;
    logMessage.source = MAIN_TASK_ID;
    logMessage.timestamp = 0;
    logMessage.data.intData = data;     /*Sending integer data will make use of the union variable*/
    logMessage.length = sizeof(data);
    if(pdPASS != xQueueSend(queue,&logMessage,portMAX_DELAY))
    {
        UARTprintf("\r\nMain Task Sending to Socket failed");
    }
    return 0;
}

int8_t logFromMain(uint8_t* data)
{
    message_t logMessage;
    logMessage.id = LOGGER;
    logMessage.source = MAIN_TASK_ID;
    logMessage.timestamp = 0;
    logMessage.data.message = data;     /*Logging will send string pointer to socket Task which inturn will store logs on BBG*/
    logMessage.length = strlen(data);
    if(pdPASS != xQueueSend(socketQueue,&logMessage,portMAX_DELAY))
    {
        UARTprintf("\r\nMain Task Sending to Socket failed");
    }
    return 0;
}

/*  ASSERT() Error function
 *
 *  failed ASSERTS() from driverlib/debug.h are executed in this function
 */
void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}
