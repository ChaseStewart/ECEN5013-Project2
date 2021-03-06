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


/*I2C Read-Write function calls*/
#define writeLightData(x,y)  (writeI2CData(I2C_LIGHT_DEVICE,x,y,LIGHT_SLAVE_ADDRESS))
#define readLightData(x,y,z) (readI2CData(I2C_LIGHT_DEVICE,x,y,z,LIGHT_SLAVE_ADDRESS))

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

/****************************************************************
* name  : writeCtrlReg
* brief : This function is writes data in to Ctrl register
* param : uint8_t data to be written
* return: 0 -Success, -1 Failure
*****************************************************************/
int8_t writeCtrlReg(uint8_t data);

/****************************************************************
* name  : readIDRegister
* brief : This function is reads data from ID register
* param : uint8_t* location to store data to be read
* return: 0 -Success, -1 Failure
*****************************************************************/
int8_t readIDRegister(uint8_t* id);

/****************************************************************
* name  : readADC0
* brief : This function is reads data from ADC0 register
* param : uint16_t* location to store data to be read
* return: 0 -Success, -1 Failure
*****************************************************************/
int8_t readADC0(uint16_t* lux);

/****************************************************************
* name  : readADC0
* brief : This function is reads data from ADC register
* param : uint16_t* location to store data to be read
* return: 0 -Success, -1 Failure
*****************************************************************/
int8_t readADC1(uint16_t* lux);

/****************************************************************
* name  : readTimingRegister
* brief : This function reads data from Timing register
* param : uint8_t* location to store data to be read
* return: 0 -Success, -1 Failure
*****************************************************************/
int8_t readTimingRegister(uint8_t* data);

/****************************************************************
* name  : lightSensorLux
* brief : This function gets the lux data
* param : float* location to store data to be read
* return: 0 -Success, -1 Failure
*****************************************************************/
int8_t lightSensorLux(float* intensity);

/****************************************************************
* name  : lightConversion
* brief : This function converts lux data
* param : CH0, CH1 - ADC value
* return: intensity data
*****************************************************************/
float lightConversion(float CH0, float CH1);

/****************************************************************
* name  : isDark
* brief : This function decides the darkness
* param : none
* return: true/false
*****************************************************************/
bool isDark();

/****************************************************************
* name  : isBright
* brief : This function decides the brightness
* param : none
* return: true/false
*****************************************************************/
bool isBright();

#endif /* LIGHTTIVA_H_ */
