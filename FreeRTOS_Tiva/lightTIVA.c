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

/*Global variable for Queue Handles in main*/
extern QueueHandle_t mainQueue;
extern QueueHandle_t tempQueue;
extern QueueHandle_t lightQueue;
extern QueueHandle_t socketQueue;
extern QueueHandle_t soilQueue;
extern QueueHandle_t chargeQueue;

extern uint32_t sysClockSet;
extern bool stateRunning;

void lightTask(void *pvParameters)
{
    message_t queueData;        /*Variable to store msgs read from queue*/
    uint32_t notificationValue = 0;
    lightSensorInit();
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
                    float data = 0;
                    lightSensorLux(&data);
                    if(data < 50)                           /*Testing Use case - FIX ME*/
                    {
                        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 1);   /*When dark turn ON Light*/
                    }
                    else
                    {
                        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
                    }
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

int8_t writeData(uint8_t regAddr, uint8_t data)
{
    /*Set Slave Address*/
    I2CMasterSlaveAddrSet(I2C_LIGHT_DEVICE, LIGHT_SLAVE_ADDRESS, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(I2C_LIGHT_DEVICE, regAddr);

    /*Start Send*/
    I2CMasterControl(I2C_LIGHT_DEVICE, I2C_MASTER_CMD_SINGLE_SEND);

    /*Wait for send complete*/
	while (!(I2CMasterBusy(I2C0_BASE)));
    while(I2CMasterBusy(I2C_LIGHT_DEVICE));

    if(I2CMasterErr(I2C_LIGHT_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        return -1;
    }

    /*Set Slave Address*/
   // I2CMasterSlaveAddrSet(I2C_LIGHT_DEVICE, LIGHT_SLAVE_ADDRESS, WRITE_FLAG);

    /*Data to be sent*/
    I2CMasterDataPut(I2C_LIGHT_DEVICE, data);

    /*Start Send*/
    I2CMasterControl(I2C_LIGHT_DEVICE, I2C_MASTER_CMD_SINGLE_SEND);

    /*Wait for send complete*/
  //  while (!(I2CMasterBusy(I2C0_BASE)));
	while(I2CMasterBusy(I2C_LIGHT_DEVICE));

    if(I2CMasterErr(I2C_LIGHT_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        return -1;
    }

    return 0;
}

int8_t readData(uint8_t regAddr, uint8_t *data, uint8_t bytes)
{
    uint32_t receivedValue = 0;

    /*Set Slave Address to write*/
    I2CMasterSlaveAddrSet(I2C_LIGHT_DEVICE, LIGHT_SLAVE_ADDRESS, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(I2C_LIGHT_DEVICE, regAddr);

    /*Start Send*/
    I2CMasterControl(I2C_LIGHT_DEVICE, I2C_MASTER_CMD_BURST_SEND_START);

    /*Wait for send complete*/
    while (!(I2CMasterBusy(I2C_LIGHT_DEVICE)));
    while(I2CMasterBusy(I2C_LIGHT_DEVICE));

    if(I2CMasterErr(I2C_LIGHT_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        //return -1;
    }

    /*Set Slave Address to Read*/
    I2CMasterSlaveAddrSet(I2C_LIGHT_DEVICE, LIGHT_SLAVE_ADDRESS, READ_FLAG);

    if(bytes == 1)
    {
        /*Start Receive*/
        I2CMasterControl(I2C_LIGHT_DEVICE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    }
    else
    {
        /*Start Receive*/
        I2CMasterControl(I2C_LIGHT_DEVICE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    }
    /*Wait for send complete*/
    while(!(I2CMasterBusy(I2C_LIGHT_DEVICE)));
    while(I2CMasterBusy(I2C_LIGHT_DEVICE));
    if(I2CMasterErr(I2C_LIGHT_DEVICE) != I2C_MASTER_ERR_NONE)
    {
       //return -1;
    }
    receivedValue = I2CMasterDataGet(I2C_LIGHT_DEVICE);
    *data = (uint8_t)receivedValue;

    if(bytes == 2)
    {
        /*Start Receive*/
        I2CMasterControl(I2C_LIGHT_DEVICE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        /*Wait for send complete*/
        while(!(I2CMasterBusy(I2C_LIGHT_DEVICE)));
        while(I2CMasterBusy(I2C_LIGHT_DEVICE));
        if(I2CMasterErr(I2C_LIGHT_DEVICE) != I2C_MASTER_ERR_NONE)
        {
           //return -1;
        }
        receivedValue = I2CMasterDataGet(I2C_LIGHT_DEVICE);
        *(data+1) = (uint8_t)receivedValue;
    }
    return 0;
}

int8_t writeCtrlReg(uint8_t data)
{
    return writeData(BIT_CMD_SELECT_REG | LIGHT_CTRL_REG, data);
}

int8_t readIDRegister(uint8_t* id)
{
    int8_t status;
    *id = 0;
    status = readData(LIGHT_ID_REG| BIT_CMD_SELECT_REG, id, 1);
    return status;
}

int8_t readADC0(uint16_t* lux)
{
    int8_t status;
    status = readData(LIGHT_DATA0LOW_REG | BIT_CMD_SELECT_REG | BIT_WORD_CMD, (uint8_t*)lux,2);
    return status;
}

int8_t readADC1(uint16_t* lux)
{
    int8_t status;
    status = readData(LIGHT_DATA1LOW_REG | BIT_CMD_SELECT_REG | BIT_WORD_CMD, (uint8_t*)lux,2);
    return status;
}

int8_t readTimingRegister(uint8_t* data)
{
    int8_t status;
    status = readData(LIGHT_TIMING_REG | BIT_CMD_SELECT_REG, data, 1);
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
