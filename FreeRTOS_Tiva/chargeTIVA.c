/****************************************************************
* file  : chargeTIVA.c
* brief : This file implements the functionalities to monitor battery charge
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#include "common.h"
#include "chargeTIVA.h"
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

void chargeTask(void *pvParameters)
{
    message_t queueData;                /*Variable to store msgs read from queue*/
    uint32_t notificationValue = 0;
    while(stateRunning)
    {
        uint32_t queueData;
        uint16_t data = 0;
        uint8_t id = 0;
        chargeSensorInit();
        chgreadData(REG_VERSION, &data, 2);
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);

        xTaskNotifyWait(0x00, ULONG_MAX, &notificationValue, portMAX_DELAY);   /*Blocks indefinitely waiting for notification*/
        if(notificationValue & TASK_NOTIFYVAL_HEARTBEAT)
        {
            sendHeartBeat(FUEL_TASK_ID);
        }
        if(notificationValue & TASK_NOTIFYVAL_MSGQUEUE)
        {
            while(errQUEUE_EMPTY != xQueueReceive(chargeQueue,(void*)&queueData,0))       /*Non-blocking call, Read Until Queue is empty*/
            {
                UARTprintf("\r\nCharge Task Received a Queue Data");
                if(queueData.id == HEARTBEAT_REQ)
                {
                    sendHeartBeat(FUEL_TASK_ID);
                }
                if(queueData.id == FUEL_GAUGE_DATA_REQ)
                {
					/*Moved Here during Merging, FIX ME if needed*/
					getChargeData(&data);
					if(data < 50)
					{
						GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 1);   /*When dark turn ON Light*/
					}
					else
					{
						GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
					}
                }
            }
        }
    }
    vTaskDelete(NULL);  /*Deletes Current task and frees up memory*/
}

void chargeSensorInit(void)
{
    int i;
    uint8_t alert_amt[2];   /* set HW interrupt to go at 32% */
    uint8_t reset[2];       /* Force a reset of the sensor */

    reset[0] = RESET_HIGH_BYTE;
    reset[1] = RESET_LOW_BYTE;

    alert_amt[0] = ALERT_HIGH_BYTE;
    alert_amt[1] = ALERT_LOW_BYTE;

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
    I2CMasterInitExpClk(I2C_DEVICE, sysClockSet, false);

    /* set sensor to reset */
     writeNBytes(REG_CONFIG_RESET, alert_amt, 2);

    /* set sensor to alarm at 32% charge */
    writeNBytes(REG_CONFIG_ALERT_PCT, reset, 2);

}


int8_t chgwriteNBytes(uint8_t regAddr, uint8_t *data, uint8_t length)
{
    uint8_t i;

    /*Set Slave Address*/
    I2CMasterSlaveAddrSet(I2C_DEVICE, CHARGE_SLAVE_ADDRESS, WRITE_FLAG);

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
    I2CMasterDataPut(I2C0_BASE, *data++);

    /*Start Send*/
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    /*Wait for send complete*/
    while(I2CMasterBusy(I2C0_BASE));

    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        return -1;
    }

    /*For n-1 times, send intermediate packets*/
    for (i=1; i<length-1; i++)
    {
        /*Data to be sent*/
        I2CMasterDataPut(I2C0_BASE, *data++);

        /*Start Send*/
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

        /*Wait for send complete*/
        while(I2CMasterBusy(I2C0_BASE));

        if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
        {
            return -1;
        }
    }

    /*Data to be sent*/
    I2CMasterDataPut(I2C0_BASE, *data++);

    /*Start Send*/
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

    /*Wait for send complete*/
    while(I2CMasterBusy(I2C0_BASE));

    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        return -1;
    }

    return 0;
}

int8_t chgwriteData(uint8_t regAddr, uint8_t data)
{
    /*Set Slave Address*/
    I2CMasterSlaveAddrSet(I2C_DEVICE, CHARGE_SLAVE_ADDRESS, WRITE_FLAG);

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
    while(I2CMasterBusy(I2C0_BASE));

    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        return -1;
    }

    return 0;
}

int8_t chgreadData(uint8_t regAddr, uint8_t *data, uint8_t bytes)
{
    uint32_t receivedValue = 0;

    /*Set Slave Address to write*/
    I2CMasterSlaveAddrSet(I2C_DEVICE, CHARGE_SLAVE_ADDRESS, WRITE_FLAG);

    /*Reg Address to be sent*/
    I2CMasterDataPut(I2C_DEVICE, regAddr);

    /*Start Send*/
    I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_SINGLE_SEND);

    /*Wait for send complete*/
    while (!(I2CMasterBusy(I2C0_BASE)));
    while(I2CMasterBusy(I2C_DEVICE));

    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
        //return -1;
    }

    /*Set Slave Address to Read*/
    I2CMasterSlaveAddrSet(I2C_DEVICE, CHARGE_SLAVE_ADDRESS, READ_FLAG);

    if(bytes == 1)
    {
        /*Start Receive*/
        I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    }
    else
    {
        /*Start Receive*/
        I2CMasterControl(I2C_DEVICE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    }
    /*Wait for send complete*/
    while(!(I2CMasterBusy(I2C_DEVICE)));
    while(I2CMasterBusy(I2C_DEVICE));
    if(I2CMasterErr(I2C_DEVICE) != I2C_MASTER_ERR_NONE)
    {
       //return -1;
    }
    receivedValue = I2CMasterDataGet(I2C_DEVICE);
    *data = (uint8_t)receivedValue;

    if(bytes == 2)
    {
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
    }
    return 0;
}

int8_t getChargeData(uint16_t *charge_amt)
{
    int8_t retval;
    uint16_t temp, high_byte, low_byte, final;


    retval = chgreadData(REG_READ_REGISTER, (uint8_t*)charge_amt, 2);
    if (retval < 0)
    {
        UARTprintf("\r\nRead failed");
        return -1;
    }
    /* split results into 2 int16_t values */
    high_byte = *charge_amt++;
    low_byte  = *charge_amt;

    /* perform transformation from datasheet. */
    temp = ((low_byte|(high_byte << 8)) >> 4 );
    final = (uint16_t ) (1.25 * temp);
    charge_amt = &final;

}
