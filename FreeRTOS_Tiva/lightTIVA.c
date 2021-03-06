/****************************************************************
* file  : lightTIVA.c
* brief : This file implements functions related to handling
*         light sensor on TIVA using FreeTOS
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#include "common.h"
#include "lightTIVA.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "i2cTIVA.h"

/*Global variable for Queue Handles in main*/
extern QueueHandle_t mainQueue;
extern QueueHandle_t tempQueue;
extern QueueHandle_t lightQueue;
extern QueueHandle_t socketQueue;
extern QueueHandle_t soilQueue;

extern uint32_t sysClockSet;
extern bool stateRunning;

void lightTask(void *pvParameters)
{
    message_t queueData;        /*Variable to store msgs read from queue*/
    uint32_t notificationValue = 0;
    float data = 0;
    lightSensorInit();      /*Turns ON the sensor*/
    lightSensorLux(&data);  /*Initializing with the current sensor value*/
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
    while(stateRunning)
    {
        xTaskNotifyWait(0x00, ULONG_MAX, &notificationValue, portMAX_DELAY);   /*Blocks indefinitely waiting for notification*/
        if(notificationValue & TASK_NOTIFYVAL_HEARTBEAT)
        {
            sendHeartBeat(LIGHT_TASK_ID);
        }
        if(notificationValue & TASK_NOTIFYVAL_MSGQUEUE)
        {
            while(errQUEUE_EMPTY != xQueueReceive(lightQueue,(void*)&queueData,0))                     /*Non-blocking call, Read Until Queue is empty*/
            {
                UARTprintf("\r\nLight Received a Queue Data");
                if(queueData.id == HEARTBEAT_REQ)
                {
                    sendHeartBeat(LIGHT_TASK_ID);
                }
                if(queueData.id == LIGHT_DATA_REQ)
                {
                    lightSensorLux(&data);
                    sendDataToMain(LIGHT_TASK_ID,LIGHT_VALUE,(int32_t)data);
                }
            }
        }
    }
    vTaskDelete(NULL);  /*Deletes Current task and frees up memory*/
}

void lightSensorInit(void)
{
    writeCtrlReg(BIT_POWER_UP);     /*Power ON the register*/
}

int8_t writeCtrlReg(uint8_t data)
{
    return writeLightData(BIT_CMD_SELECT_REG | LIGHT_CTRL_REG, data);
}

int8_t readIDRegister(uint8_t* id)
{
    int8_t status;
    *id = 0;
    status = readLightData(LIGHT_ID_REG| BIT_CMD_SELECT_REG, id, 1);
    return status;
}

int8_t readADC0(uint16_t* lux)
{
    int8_t status;
    status = readLightData(LIGHT_DATA0LOW_REG | BIT_CMD_SELECT_REG | BIT_WORD_CMD, (uint8_t*)lux,2);
    return status;
}

int8_t readADC1(uint16_t* lux)
{
    int8_t status;
    status = readLightData(LIGHT_DATA1LOW_REG | BIT_CMD_SELECT_REG | BIT_WORD_CMD, (uint8_t*)lux,2);
    return status;
}

int8_t readTimingRegister(uint8_t* data)
{
    int8_t status;
    status = readLightData(LIGHT_TIMING_REG | BIT_CMD_SELECT_REG, data, 1);
    return status;
}

/*Function to know the Sensor Lux Value*/
int8_t lightSensorLux(float* intensity)
{
    uint16_t iCH0 = 0;
    uint16_t iCH1 = 0;
    float CH0 = 0;
    float CH1 = 0;
    int8_t status;
    status = readADC0(&iCH0);
    status = readADC1(&iCH1);
    CH0 = iCH0;
    CH1 = iCH1;
    *intensity = lightConversion(CH0, CH1);
    return status;
}

float lightConversion(float CH0, float CH1)
{
    float intensity = 0;
    float selectFormula = 0;
    selectFormula = CH1/CH0;    /*Lux calculation depends on this ratio*/
    /*Formula Selection*/
    if(selectFormula <= 0.5)
    {
        intensity =  (0.0304*CH0)-(0.062*CH0*((CH1/CH0)*1.4));
    }
    else if(selectFormula <= 0.61)
    {
        intensity = (0.0224*CH0)-(0.031*CH1);
    }
    else if(selectFormula <= 0.8)
    {
        intensity =  (0.0128*CH0)-(0.0153*CH1);
    }
    else if(selectFormula <= 1.3)
    {
        intensity =  (0.00146*CH0)-(0.00112*CH1);
    }
    else
    {
        intensity = 0;
    }
    return intensity;
}

/*Function to find if the light is dark*/
bool isDark()
{
    float intensity;
    if(lightSensorLux(&intensity))
    {
        return false;
    }
    if(intensity < 50)  /*Lux value of <50 is assumed night*/
    {
        return true;
    }
    else
    {
        return false;
    }
}


/*Function to find if the light is dark*/
bool isBright()
{
    float intensity;
    if(lightSensorLux(&intensity))
    {
        return false;
    }
    if(intensity > 50)
    {
        return true;
    }
    else
    {
        return false;
    }
}
