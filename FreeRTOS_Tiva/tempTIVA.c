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

int8_t writeTempData(uint8_t regAddr, uint8_t data)
{
    /*Set Slave Address*/
    I2CMasterSlaveAddrSet(I2C_TEMP_DEVICE, TEMP_SLAVE_ADDRESS, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(I2C_TEMP_DEVICE, regAddr);

    /*Start Send*/
    I2CMasterControl(I2C_TEMP_DEVICE, I2C_MASTER_CMD_SINGLE_SEND);

    /*Wait for send complete*/
    while (!(I2CMasterBusy(I2C_TEMP_DEVICE)));
    while(I2CMasterBusy(I2C_TEMP_DEVICE));

    if(I2CMasterErr(I2C_TEMP_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        return -1;
    }

    /*Set Slave Address*/
   // I2CMasterSlaveAddrSet(I2C_LIGHT_DEVICE, LIGHT_SLAVE_ADDRESS, WRITE_FLAG);

    /*Data to be sent*/
    I2CMasterDataPut(I2C_TEMP_DEVICE, data);

    /*Start Send*/
    I2CMasterControl(I2C_TEMP_DEVICE, I2C_MASTER_CMD_SINGLE_SEND);

    /*Wait for send complete*/
  //  while (!(I2CMasterBusy(I2C0_BASE)));
    while(I2CMasterBusy(I2C_TEMP_DEVICE));

    if(I2CMasterErr(I2C_TEMP_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        return -1;
    }

    return 0;
}

int8_t readTempData(uint8_t regAddr, uint8_t *data, uint8_t bytes)
{
    uint32_t receivedValue = 0;

    /*Set Slave Address to write*/
    I2CMasterSlaveAddrSet(I2C_TEMP_DEVICE, TEMP_SLAVE_ADDRESS, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(I2C_TEMP_DEVICE, regAddr);

    /*Start Send*/
    I2CMasterControl(I2C_TEMP_DEVICE, I2C_MASTER_CMD_BURST_SEND_START);

    /*Wait for send complete*/
    while (!(I2CMasterBusy(I2C_TEMP_DEVICE)));
    while(I2CMasterBusy(I2C_TEMP_DEVICE));

    if(I2CMasterErr(I2C_TEMP_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        //return -1;
    }

    /*Set Slave Address to Read*/
    I2CMasterSlaveAddrSet(I2C_TEMP_DEVICE, TEMP_SLAVE_ADDRESS, READ_FLAG);

    if(bytes == 1)
    {
        /*Start Receive*/
        I2CMasterControl(I2C_TEMP_DEVICE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    }
    else
    {
        /*Start Receive*/
        I2CMasterControl(I2C_TEMP_DEVICE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    }
    /*Wait for send complete*/
    while(!(I2CMasterBusy(I2C_TEMP_DEVICE)));
    while(I2CMasterBusy(I2C_TEMP_DEVICE));
    if(I2CMasterErr(I2C_TEMP_DEVICE) != I2C_MASTER_ERR_NONE)
    {
       //return -1;
    }
    receivedValue = I2CMasterDataGet(I2C_TEMP_DEVICE);
    *data = (uint8_t)receivedValue;

    if(bytes == 2)
    {
        /*Start Receive*/
        I2CMasterControl(I2C_TEMP_DEVICE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        /*Wait for send complete*/
        while(!(I2CMasterBusy(I2C_TEMP_DEVICE)));
        while(I2CMasterBusy(I2C_TEMP_DEVICE));
        if(I2CMasterErr(I2C_TEMP_DEVICE) != I2C_MASTER_ERR_NONE)
        {
           //return -1;
        }
        receivedValue = I2CMasterDataGet(I2C_TEMP_DEVICE);
        *(data+1) = (uint8_t)receivedValue;
    }
    return 0;
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
    status = readTempData(PTR_ADDRESS_TEMP,(uint8_t*)temp,2);
    *temp = ((*temp >> 8) | ((*temp & 0x00FF) << 8));  /*Expected First Byte and second byte are reversed, so changing the Endianess*/
    *temp = tempConversion(*temp);
    return status;
}



