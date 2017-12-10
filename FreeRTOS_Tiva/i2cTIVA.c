/*
 * i2cTIVA.c
 *
 *  Created on: Dec 6, 2017
 *      Author: cstewart
 */

#include "common.h"
#include "i2cTIVA.h"
#include "semphr.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"

extern SemaphoreHandle_t I2CMutex;

void myI2CInit(void)
{
    /*Enable I2C0*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0));

    /*Enable GPIO*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    /*Configure GPIOs for I2C*/
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);                /*Pin configure must be called for each pin*/
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);

    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);       /*I2C0 - SDA pin*/
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);    /*I2C0 - SCL pin*/

    /*Set Clock Speed and enable Master*/
    I2CMasterInitExpClk(I2C0_BASE, SYSTEM_CLOCK, false);
}


int8_t writeI2CData(uint32_t i2cDevice, uint8_t regAddr, uint8_t data, uint8_t slaveAddress)
{
    xSemaphoreTake(I2CMutex, portMAX_DELAY);
    taskDISABLE_INTERRUPTS();
    /*Set Slave Address*/
    I2CMasterSlaveAddrSet(i2cDevice, slaveAddress, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(i2cDevice, regAddr);

    /*Start Send*/
    I2CMasterControl(i2cDevice, I2C_MASTER_CMD_SINGLE_SEND);

    /*Wait for send complete*/
    while (!(I2CMasterBusy(i2cDevice)));
    while(I2CMasterBusy(i2cDevice));

    if(I2CMasterErr(i2cDevice) != I2C_MASTER_ERR_NONE)
    {
    }

    /*Data to be sent*/
    I2CMasterDataPut(i2cDevice, data);

    /*Start Send*/
    I2CMasterControl(i2cDevice, I2C_MASTER_CMD_SINGLE_SEND);

    /*Wait for send complete*/
  //  while (!(I2CMasterBusy(I2C0_BASE)));
    while(I2CMasterBusy(i2cDevice));

    if(I2CMasterErr(i2cDevice) != I2C_MASTER_ERR_NONE)
    {
    }
    taskENABLE_INTERRUPTS();
    xSemaphoreGive(I2CMutex);
    return 0;
}

int8_t readI2CData(uint32_t i2cDevice, uint8_t regAddr, uint8_t *data, uint8_t bytes, uint8_t slaveAddress)
{
    xSemaphoreTake(I2CMutex, portMAX_DELAY);
    taskDISABLE_INTERRUPTS();
    uint32_t receivedValue = 0;

    /*Set Slave Address to write*/
    I2CMasterSlaveAddrSet(i2cDevice, slaveAddress, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(i2cDevice, regAddr);

    /*Start Send*/
    I2CMasterControl(i2cDevice, I2C_MASTER_CMD_BURST_SEND_START);

    /*Wait for send complete*/
    while (!(I2CMasterBusy(i2cDevice)));
    while(I2CMasterBusy(i2cDevice));

    if(I2CMasterErr(i2cDevice) != I2C_MASTER_ERR_NONE)
    {
        //return -1;
    }

    /*Set Slave Address to Read*/
    I2CMasterSlaveAddrSet(i2cDevice, slaveAddress, READ_FLAG);

    if(bytes == 1)
    {
        /*Start Receive*/
        I2CMasterControl(i2cDevice, I2C_MASTER_CMD_SINGLE_RECEIVE);
    }
    else
    {
        /*Start Receive*/
        I2CMasterControl(i2cDevice, I2C_MASTER_CMD_BURST_RECEIVE_START);
    }
    /*Wait for send complete*/
    while(!(I2CMasterBusy(i2cDevice)));
    while(I2CMasterBusy(i2cDevice));
    if(I2CMasterErr(i2cDevice) != I2C_MASTER_ERR_NONE)
    {
       //return -1;
    }
    receivedValue = I2CMasterDataGet(i2cDevice);
    *data = (uint8_t)receivedValue;

    if(bytes == 2)
    {
        /*Start Receive*/
        I2CMasterControl(i2cDevice, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        /*Wait for send complete*/
        while(!(I2CMasterBusy(i2cDevice)));
        while(I2CMasterBusy(i2cDevice));
        if(I2CMasterErr(i2cDevice) != I2C_MASTER_ERR_NONE)
        {
           //return -1;
        }
        receivedValue = I2CMasterDataGet(i2cDevice);
        *(data+1) = (uint8_t)receivedValue;
    }
    taskENABLE_INTERRUPTS();
    xSemaphoreGive(I2CMutex);
    return 0;
}
