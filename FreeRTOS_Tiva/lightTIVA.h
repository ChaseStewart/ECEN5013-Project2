/****************************************************************
* file  : lightTIVA.h
* brief : This file implements functions related to handling
*         light sensor on TIVA using FreeTOS
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef LIGHTTIVA_H_
#define LIGHTTIVA_H_

/*Macros*/
#define LIGHT_SLAVE_ADDRESS     0x39            /*Slave Address*/
#define I2C_LIGHT_DEVICE        I2C0_BASE
/*Address of registers*/
#define LIGHT_CTRL_REG          0x00
#define LIGHT_TIMING_REG        0x01
#define LIGHT_THR_LOW_LOW_REG   0x02
#define LIGHT_THR_LOW_HIGH_REG  0x03
#define LIGHT_THR_HIGH_LOW_REG  0x04
#define LIGHT_THR_HIGH_HIGH_REG 0x05
#define LIGHT_INT_REG           0x06
#define LIGHT_ID_REG            0x0A
#define LIGHT_DATA0LOW_REG      0x0C
#define LIGHT_DATA0HIGH_REG     0x0D
#define LIGHT_DATA1LOW_REG      0x0E
#define LIGHT_DATA1HIGH_REG     0x0F

/*Light Sensor Commands*/
#define BIT_WORD_CMD            0x20
#define BIT_CMD_SELECT_REG      0x80
#define BIT_POWER_UP            0x03
#define BIT_POWER_DOWN          0x00
#define BIT_INT_TIME_14ms       0x00
#define BIT_INT_TIME_101ms      0x01
#define BIT_INT_TIME_402ms      0x10
#define BIT_INT_DISABLE         0x00
#define BIT_INT_ENABLE          0x01

#define WRITE_FLAG  false
#define READ_FLAG   true

/****************************************************************
* name  : lightTask
* brief : This function is the start of the Light Tasks which
*         handles Light sensor
* param : Passed when creating the task
* return: None
*****************************************************************/
void lightTask(void *pvParameters);

/****************************************************************
* name  : lightSensorInit
* brief : This function is initializes the I2C peripheral for I2C sensor
* param : None
* return: None
*****************************************************************/
void lightSensorInit(void);

int8_t writeData(uint8_t regAddr, uint8_t data);

int8_t readData(uint8_t regAddr, uint8_t *data, uint8_t bytes);

int8_t writeCtrlReg(uint8_t data);

int8_t readIDRegister(uint8_t* id);

int8_t readADC0(uint16_t* lux);

int8_t readADC1(uint16_t* lux);

int8_t readTimingRegister(uint8_t* data);

int8_t lightSensorLux(float* intensity);

float lightConversion(float CH0, float CH1);

bool isDark();

bool isBright();

#endif /* LIGHTTIVA_H_ */
