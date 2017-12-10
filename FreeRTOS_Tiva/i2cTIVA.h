/****************************************************************
* file  : i2cTIVA.h
* brief : This file implements the general read and write functionalities for I2C0
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/


#ifndef I2CTIVA_H_
#define I2CTIVA_H_

/* I2C defs */
#define WRITE_FLAG          false
#define READ_FLAG           true

/****************************************************************
* name  : I2CInit
* brief : Initialize the I2C bus for fuel gauge, temp sensor, and light sensor
* param : None
* return: None
*****************************************************************/
void myI2CInit(void);

/****************************************************************
* name  : writeI2CData
* brief : use I2C to write 1 byte to provided register
* param : uint32_t i2cDevice - Base address of the I2C device
* param : uint8_t slaveAddress - the slave register for the sensor
* param : uint8_t regAddr   - the I2C register into which to write
* param : uint8_t data      - a pointer to the data that should be written
* return: int8_t status     - 0 is success else failure
*****************************************************************/
int8_t writeI2CData(uint32_t i2cDevice, uint8_t regAddr, uint8_t data, uint8_t slaveAddress);

/****************************************************************
* name  : readNBytes
* brief : use I2C to read N bytes from provided register
* param : uint8_t slaveAddr - the slave register for the sensor
* param : uint8_t regAddr   - the I2C register from which to read
* param : uint8_t *data     - a pointer to the variable to receive the data
* param : uint8_t bytes     - the number of bytes of data
* return: int8_t status     - 0 is success else failure
*****************************************************************/
int8_t readI2CData(uint32_t i2cDevice, uint8_t regAddr, uint8_t *data, uint8_t bytes, uint8_t slaveAddress);

#endif /* I2CTIVA_H_ */
