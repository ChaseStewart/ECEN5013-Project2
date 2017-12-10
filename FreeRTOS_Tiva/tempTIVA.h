/****************************************************************
* file  : tempTIVA.h
* brief : This file implements functions related to handling
*         temperature sensor on TIVA using FreeTOS
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef TEMPTIVA_H_
#define TEMPTIVA_H_

/*Macros for Temperature Sensor*/
#define I2C_TEMP_DEVICE         I2C0_BASE
#define TEMP_SLAVE_ADDRESS      0x48            /*Slave Address*/
#define TEMP_SENSOR_RESOLUTION  0.0625          /*ADC Conversion Resolution*/
#define READ_BIT                1
#define WRITE_BIT               0

#define PTR_ADDRESS_TEMP        0x00            /*Temperature REgister*/
#define PTR_ADDRESS_CONFIG      0x01            /*Congig Register*/
#define PTR_ADDRESS_TLOW        0x02            /*Low Threshold Register*/
#define PTR_ADDRESS_THIGH       0x03            /*High Threshold Register*/

#define BIT_SHUTDOWN_16BIT      0x0100          /*Shutdown Bits set <Byte1Byte2> format*/
#define BIT_CONV_RATE_0_25HZ    0x00            /*Conversion Rate*/
#define BIT_CONV_RATE_1HZ       0x40
#define BIT_CONV_RATE_4HZ       0x80
#define BIT_CONV_RATE_8HZ       0xC0


/****************************************************************
* name  : tempTask
* brief : This function is the start of the Temperature Tasks which
*         handles temperature
* param : Passed when creating the task
* return: None
*****************************************************************/
void tempTask(void *pvParameters);

/****************************************************************
* name  : writeTempData
* brief : This function write data into the sensor's registers
* param : refAddr - Register to
* return: 0 - Success, -1 on Failure
*****************************************************************/
int8_t writeTempData(uint8_t regAddr, uint8_t data);


int8_t readTempData(uint8_t regAddr, uint8_t *data, uint8_t bytes);

int16_t tempConversion(int16_t temp);

int8_t readTemperature(int16_t* temp);

#endif /* TEMPTIVA_H_ */
