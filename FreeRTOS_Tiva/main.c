/****************************************************************
* file  : main.c
* brief : This file implements main task functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/
#include "main.h"
#include "lightTIVA.h"
#include "tempTIVA.h"
#include "socketTIVA.h"
#include "soilTIVA.h"
#include "i2cTIVA.h"
#include "semphr.h"
#include "driverlib/adc.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include <string.h>
#include <stdio.h>

/*Global Variables*/
/*Queue Handles*/
QueueHandle_t mainQueue;
QueueHandle_t tempQueue;
QueueHandle_t lightQueue;
QueueHandle_t socketQueue;
QueueHandle_t soilQueue;

/*Task state variables*/
bool stateRunning = true;

/*Task Handles*/
TaskHandle_t lightTaskHandle;
TaskHandle_t tempTaskHandle;
TaskHandle_t soilTaskHandle;
TaskHandle_t mainTaskHandle;
TaskHandle_t socketTaskHandle;

uint32_t sysClockSet = 0;

/*Timer Handle*/
TimerHandle_t hbTimerHandle;
TimerHandle_t wdTimerHandle;

/*Mutex*/
SemaphoreHandle_t I2CMutex;

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
    UARTStdioConfig(0, 115200, SYSTEM_CLOCK);

    /*Initializing the mutex*/
    I2CMutex = xSemaphoreCreateMutex();
    if(I2CMutex == NULL)
    {
        UARTprintf("\r\nMutex Initialization Failed");
    }
    /*Creating Queues*/
    mainQueue = xQueueCreate(MAIN_QUEUE_SIZE, sizeof(message_t));
    if(mainQueue == NULL)
    {
       UARTprintf("\r\nMain Queue Creation Failed");
    }

    UARTprintf("\r\n\n*** Begin project ***");

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
    soilQueue = xQueueCreate(SOIL_QUEUE_SIZE, sizeof(message_t));
    if(soilQueue == NULL)
    {
        UARTprintf("\r\nSoil Queue Creation Failed");
    }

    /*Create a timer for heart-beats*/
    hbTimerHandle = xTimerCreate("HBTimer",pdMS_TO_TICKS(3000),pdTRUE,(void*)0,hbTimerCB);
    if( hbTimerHandle == NULL )
    {
        UARTprintf("\r\nTimer Creation Failed");
        return -1;
    }
    /*Create a timer for watchdog*/
    wdTimerHandle = xTimerCreate("WDTimer",pdMS_TO_TICKS(6000),pdTRUE,(void*)0,wdTimerCB);  /*This timer checks if all the tasks has sent the notification*/
    if( wdTimerHandle == NULL )
    {
       UARTprintf("\r\nTimer Creation Failed");
       return -1;
    }
    stateRunning = true;

    myI2CInit();
    myADCInit();

    /*Create different Tasks*/

    if(xTaskCreate(mainTask, (const portCHAR *)"MainTask", configMINIMAL_STACK_SIZE, NULL, 1, &mainTaskHandle) != pdPASS)
    {
        UARTprintf("\r\nMain Task creation failed");
        stateRunning = false;
        return -1;
    }

    if(xTaskCreate(lightTask, (const portCHAR *)"LightTask", configMINIMAL_STACK_SIZE, NULL, 3, &lightTaskHandle) != pdPASS)
    {
        UARTprintf("\r\nLight Task creation failed");
        stateRunning = false;
        return -1;
    }
    if(xTaskCreate(tempTask, (const portCHAR *)"TemperatureTask", configMINIMAL_STACK_SIZE, NULL, 4, &tempTaskHandle) != pdPASS)
    {
        UARTprintf("\r\nTemperature Task creation failed");
        stateRunning = false;
        return -1;
    }

    if(xTaskCreate(socketTask, (const portCHAR *)"SocketTask", configMINIMAL_STACK_SIZE, NULL, 5, &socketTaskHandle) != pdPASS)
    {
        UARTprintf("\r\nSocket Task creation failed");
        stateRunning = false;
        return -1;
    }

    if(xTaskCreate(soilTask, (const portCHAR *)"SoilTask", configMINIMAL_STACK_SIZE, NULL, 2, &soilTaskHandle) != pdPASS)
    {
        UARTprintf("\r\nTemperature Task creation failed");
        stateRunning = false;
        return -1;
    }

    /*Start the scheduler*/
    vTaskStartScheduler();

    return 0;
}

void mainTask(void *pvParameters)
{
    message_t queueData;        /*Variable to store msgs read from queue*/
    uint32_t notificationValue = 0;
    uint32_t hbFlags = 0;
    uint32_t tempData = 0;
    uint32_t lightData = 0;
    uint32_t soilData = 0;
    uint8_t socketMsg[20];  /*For sending to Socket*/
    /*Start Timer for Hear-beat Requests*/
    if( xTimerStart( hbTimerHandle, portMAX_DELAY ) != pdPASS )
    {
        UARTprintf("\r\nTimer starting failed");
        stateRunning = false;
        return;
    }

    /*Starts a timer for checking heartbeats for once in 6seconds*/
    if( xTimerStart( wdTimerHandle, portMAX_DELAY ) != pdPASS )
    {
        UARTprintf("\r\nTimer starting failed");
        stateRunning = false;
        return;
    }
    while(stateRunning)
    {
       xTaskNotifyWait(0x00, ULONG_MAX, &notificationValue, portMAX_DELAY);   /*Blocks indefinitely waiting for notification*/
       if(notificationValue & TASK_NOTIFYVAL_HEARTBEAT)
       {
           if(hbFlags == (HB_OK_LIGHT | HB_OK_TEMP | HB_OK_SOCKET | HB_OK_SOIL))
           {
               UARTprintf("\r\nSystem is in safe state");
               hbFlags = 0;
           }
           else
           {
               //stateRunning = false;
               UARTprintf("\r\nSystem lost a heart-beat");
               sprintf(socketMsg,"%d:TIVA missed a HEARTBEAT\0",ERR_PKT);
               logFromMain(socketMsg);
               hbFlags = 0;
           }
       }
       if(notificationValue & TASK_NOTIFYVAL_MSGQUEUE)
       {
           while(errQUEUE_EMPTY != xQueueReceive(mainQueue,(void*)&queueData,0))                     /*Non-blocking call, Read Until Queue is empty*/
           {
               //UARTprintf("\r\nMain Task Received a Queue Data");
               if(queueData.id == HEARTBEAT_RSP)
               {
                   if(queueData.source == LIGHT_TASK_ID)
                   {
                       hbFlags |= HB_OK_LIGHT;
                   }
                   else if(queueData.source == TEMP_TASK_ID)
                   {
                       hbFlags |= HB_OK_TEMP;
                   }
                   else if(queueData.source == SOCKET_TASK_ID)
                   {
                       hbFlags |= HB_OK_SOCKET;
                   }
                   else if(queueData.source == SOIL_TASK_ID)
                   {
                       hbFlags |= HB_OK_SOIL;
                   }
               }
               else if(queueData.id == TEMP_VALUE)
               {
                   tempData = queueData.data.intData;
               }
               else if(queueData.id == LIGHT_VALUE)
               {
                   lightData = queueData.data.intData;
               }
               else if(queueData.id == SOIL_MOIST_DATA)
               {
                   soilData = queueData.data.intData;
               }
           }
       }
       if(notificationValue & TASK_NOTIFYVAL_REQDATA)
       {
           /*Send Temp data req and notify Temp task*/
           sendDataFromMain(tempQueue, TEMP_DATA_REQ, 0);
           xTaskNotify(tempTaskHandle,TASK_NOTIFYVAL_MSGQUEUE, eSetBits);

           /*Send Light Data Req*/
           sendDataFromMain(lightQueue, LIGHT_DATA_REQ, 0);
           xTaskNotify(lightTaskHandle,TASK_NOTIFYVAL_MSGQUEUE, eSetBits);

           /*Send Soil Data Req*/
           sendDataFromMain(soilQueue, SOIL_MOIST_DATA_REQ, 0);
           xTaskNotify(soilTaskHandle,TASK_NOTIFYVAL_MSGQUEUE, eSetBits);
       }
       if(notificationValue & TASK_NOTIFYVAL_SENDTOSCKT)
       {
           sprintf(socketMsg,"%d:%d:%d:%d\0",DATA_PKT,tempData,lightData,soilData);
           logFromMain(socketMsg);
       }
    }
    /*Delete all the queues that were created*/
    vQueueDelete(mainQueue);
    vQueueDelete(tempQueue);
    vQueueDelete(lightQueue);
    vQueueDelete(socketQueue);
    vQueueDelete(soilQueue);

    /*Delete Timer*/
    xTimerDelete(hbTimerHandle,portMAX_DELAY);
    xTimerDelete(wdTimerHandle,portMAX_DELAY);

    vTaskDelete(NULL);  /*Deletes Current task and frees up memory*/
    return;
}


int8_t sendDataFromMain(QueueHandle_t queue, Message_Type msgID, int32_t data)
{
    message_t message;
    message.id = msgID;
    message.source = MAIN_TASK_ID;
    message.timestamp = 0;
    message.data.intData = data;     /*Sending integer data will make use of the union variable*/
    message.length = sizeof(data);
    if(pdPASS != xQueueSend(queue,(void*)&message,portMAX_DELAY))
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
    if(pdPASS != xQueueSend(socketQueue,(void*)&logMessage,portMAX_DELAY))
    {
        UARTprintf("\r\nMain Task Sending to Socket failed");
    }
    xTaskNotify(socketTaskHandle,TASK_NOTIFYVAL_MSGQUEUE, eSetBits);
    return 0;
}

void hbTimerCB(TimerHandle_t xTimer)
{
    /*Sends heartbeat request when the timer expires*/
    xTaskNotify(lightTaskHandle,TASK_NOTIFYVAL_HEARTBEAT, eSetBits); /*Always will return pdPASS in this use-case*/
    xTaskNotify(tempTaskHandle,TASK_NOTIFYVAL_HEARTBEAT, eSetBits);
    xTaskNotify(socketTaskHandle,TASK_NOTIFYVAL_HEARTBEAT, eSetBits);
    xTaskNotify(soilTaskHandle,TASK_NOTIFYVAL_HEARTBEAT, eSetBits);

}

void wdTimerCB(TimerHandle_t xTimer)
{
    static int8_t count = 0;
    count++;
    xTaskNotify(mainTaskHandle,TASK_NOTIFYVAL_HEARTBEAT, eSetBits); /*sends a notification to main to check for Heart-beats*/
    //if((count % 3) == 0)
    {
        xTaskNotify(mainTaskHandle,TASK_NOTIFYVAL_SENDTOSCKT, eSetBits); /*send data to BBG*/
    }
    if((count % 3) == 0)
    {
        xTaskNotify(mainTaskHandle,TASK_NOTIFYVAL_REQDATA, eSetBits);   /*send notification to collect sensor data*/
    }

}

void myADCInit(void)
{
    /* Enable and reset the ADC peripheral*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
    {
    }
    SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    /* Configure the ADC capture sequence*/
    ADCSequenceDisable(ADC0_BASE, SOIL_SEQ_NO);
    ADCSequenceConfigure(ADC0_BASE, SOIL_SEQ_NO, ADC_TRIGGER_ALWAYS, 0);
    ADCSequenceStepConfigure(ADC0_BASE, SOIL_SEQ_NO, 0, (ADC_CTL_CH0 |  ADC_CTL_END));//ADC_CTL_IE |));

    /* Setup ADC pin and interrupts */
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    ADCIntDisable(ADC0_BASE, SOIL_SEQ_NO);
    ADCSequenceEnable(ADC0_BASE,SOIL_SEQ_NO);
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
