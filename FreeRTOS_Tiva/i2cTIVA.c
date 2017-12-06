/*
 * i2cTIVA.c
 *
 *  Created on: Dec 6, 2017
 *      Author: cstewart
 */

#include "common.h"
#include "i2cTIVA.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"

int8_t writeOneByte(uint8_t slaveAddr, uint8_t regAddr, uint8_t data)
{
    /*Set Slave Address*/
    I2CMasterSlaveAddrSet(I2C_DEVICE, slaveAddr, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(I2C_DEVICE, regAddr);

    /*Start Send*/
    I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_SINGLE_SEND);

    /*Wait for send complete*/
    while (!(I2CMasterBusy(I2C0_BASE)));
    while(I2CMasterBusy(I2C_DEVICE));

    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        return -1;
    }

    /*Data to be sent*/
    I2CMasterDataPut(I2C0_BASE, data);

    /*Start Send*/
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    /*Wait for send complete*/
  //  while (!(I2CMasterBusy(I2C0_BASE)));
    while(I2CMasterBusy(I2C0_BASE));

    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        return -1;
    }

    return 0;
}

int8_t writeNBytes(uint8_t slaveAddr, uint8_t regAddr, uint8_t *data, uint8_t bytes)
{
    /* TODO FIXME write this */
    return 0;
}

int8_t readOneByte(uint8_t slaveAddr, uint8_t regAddr, uint8_t *data)
{
    uint32_t receivedValue = 0;

    /*Set Slave Address to write*/
    I2CMasterSlaveAddrSet(I2C_DEVICE, slaveAddr, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(I2C_DEVICE, regAddr);

    /*Start Send*/
    I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_BURST_SEND_START);

    /*Wait for send complete*/
    while (!(I2CMasterBusy(I2C0_BASE)));
    while(I2CMasterBusy(I2C_DEVICE));

    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        //return -1;
    }

    /*Set Slave Address to Read*/
    I2CMasterSlaveAddrSet(I2C_DEVICE, slaveAddr, READ_FLAG);

    /*Start Receive*/
    I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    /*Wait for send complete*/
    while(!(I2CMasterBusy(I2C_DEVICE)));
    while(I2CMasterBusy(I2C_DEVICE));
    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
       //return -1;
    }
    receivedValue = I2CMasterDataGet(I2C_DEVICE);
    *data = (uint8_t)receivedValue;

    return 0;
}

int8_t readNBytes(uint8_t slaveAddr, uint8_t regAddr, uint8_t *data, uint8_t bytes)
{
    uint8_t idx;
    uint32_t receivedValue = 0;

    /*Set Slave Address to write*/
    I2CMasterSlaveAddrSet(I2C_DEVICE, slaveAddr, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(I2C_DEVICE, regAddr);

    /*Start Send*/
    I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_BURST_SEND_START);

    /*Wait for send complete*/
    while (!(I2CMasterBusy(I2C0_BASE)));
    while(I2CMasterBusy(I2C_DEVICE));

    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        //return -1;
    }

    /*Set Slave Address to Read*/
    I2CMasterSlaveAddrSet(I2C_DEVICE, slaveAddr, READ_FLAG);

    /*Start Receive*/
    I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_BURST_RECEIVE_START);

    /*Wait for send complete*/
    while(!(I2CMasterBusy(I2C_DEVICE)));
    while(I2CMasterBusy(I2C_DEVICE));
    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
       //return -1;
    }
    receivedValue = I2CMasterDataGet(I2C_DEVICE);
    *data = (uint8_t)receivedValue;

    for (idx = 1; idx < bytes-1; idx++)
    {
        /*Start Receive*/
        I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        /*Wait for send complete*/
        while(!(I2CMasterBusy(I2C0_BASE)));
        while(I2CMasterBusy(I2C_DEVICE));
        if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
        {
           //return -1;
        }
        receivedValue = I2CMasterDataGet(I2C_DEVICE);
        *(data+1) = (uint8_t)receivedValue;
    }
    /*Start Receive*/
    I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    /*Wait for send complete*/
    while(!(I2CMasterBusy(I2C0_BASE)));
    while(I2CMasterBusy(I2C_DEVICE));
    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
       //return -1;
    }
    receivedValue = I2CMasterDataGet(I2C_DEVICE);
    *(data+1) = (uint8_t)receivedValue;

    return 0;
}
