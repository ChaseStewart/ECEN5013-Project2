/****************************************************************
* file  : tempTIVA.c
* brief : This file implements functions related to handling
*         temperature sensor on TIVA using FreeTOS
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#include "common.h"
#include "tempTIVA.h"
#include "driverlib/i2c.h"
#include "i2cTIVA.h"

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
    int16_t temp = 0;

    while(stateRunning)
    {
        xTaskNotifyWait(0x00, ULONG_MAX, &notificationValue, portMAX_DELAY);   /*Blocks indefinitely waiting for notification*/
        if(notificationValue & TASK_NOTIFYVAL_HEARTBEAT)
        {
           sendHeartBeat(TEMP_TASK_ID);
           //readTemperature(&temp);
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
                    readTemperature(&temp);
                }
            }
        }
    }
    vTaskDelete(NULL);  /*Deletes Current task and frees up memory*/
}

/*Function to convert the Tmp102 ADC values to Temperature*/
int16_t tempConversion(int16_t temp)
{
    temp = temp >>4;
    if(temp & 0x800) /*finding Negative values in 12-bit ADC result*/
    {
        temp = ~temp;   /*Finding Absolute Value by 2s Complement and multpily by -1*/
        temp++;
        return (int16_t)((float)temp*TEMP_SENSOR_RESOLUTION*(-1.0));
    }
    else    /*If Positive Values*/
    {
        return (int16_t)((float)temp*TEMP_SENSOR_RESOLUTION);
    }
}

/*Function to read the temperature, returns -1 on error and 0 on success*/
int8_t readTemperature(int16_t* temp)
{
    int8_t status;
    int16_t tempCheck;
    status = readTempData(PTR_ADDRESS_TEMP,(uint8_t*)&tempCheck,2);
    tempCheck = ((tempCheck >> 8) | ((tempCheck & 0x00FF) << 8));  /*Expected First Byte and second byte are reversed, so changing the Endianess*/
    tempCheck = tempConversion(tempCheck);
    if(tempCheck == 0)
    {
      /*Invalid Temp, a work-around*/
       return -1;
    }
    else
    {
        *temp = tempCheck;
    }
    return status;
}



