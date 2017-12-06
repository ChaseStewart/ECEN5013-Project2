/****************************************************************
* file  : i2cTIVA.h
* brief : This file implements the general read and write functionalities for I2C0
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/


#ifndef I2CTIVA_H_
#define I2CTIVA_H_

/* Hardware */
#define I2C_DEVICE          I2C0_BASE

/* I2C defs */
#define WRITE_FLAG          false
#define READ_FLAG           true

/****************************************************************
* name  : writeOneByte
* brief : use I2C to write 1 byte to provided register
* param : uint8_t slaveAddr - the slave register for the sensor
* param : uint8_t regAddr   - the I2C register into which to write
* param : uint8_t data      - a pointer to the data that should be written
* return: int8_t status     - 0 is success else failure
*****************************************************************/
int8_t writeOneByte(uint8_t slaveAddr, uint8_t regAddr, uint8_t data);

/****************************************************************
* name  : writeNBytes
* brief : use I2C to write N bytes to provided register
* param : uint8_t slaveAddr - the slave register for the sensor
* param : uint8_t regAddr   - the I2C register into which to write
* param : uint8_t *data     - the data that should be written
* param : uint8_t bytes     - the number of bytes of data
* return: int8_t status     - 0 is success else failure
*****************************************************************/
int8_t writeNBytes(uint8_t slaveAddr, uint8_t regAddr, uint8_t *data, uint8_t bytes);

/****************************************************************
* name  : readOneByte
* brief : use I2C to read 1 byte from provided register
* param : uint8_t slaveAddr - the slave register for the sensor
* param : uint8_t regAddr   - the I2C register from which to read
* param : uint8_t data      - a pointer to the variable to receive the data
* return: int8_t status     - 0 is success else failure
*****************************************************************/
int8_t readOneByte(uint8_t slaveAddr, uint8_t regAddr, uint8_t *data);

/****************************************************************
* name  : readNBytes
* brief : use I2C to read N bytes from provided register
* param : uint8_t slaveAddr - the slave register for the sensor
* param : uint8_t regAddr   - the I2C register from which to read
* param : uint8_t *data     - a pointer to the variable to receive the data
* param : uint8_t bytes     - the number of bytes of data
* return: int8_t status     - 0 is success else failure
*****************************************************************/
int8_t readNBytes(uint8_t slaveAddr, uint8_t regAddr, uint8_t *data, uint8_t bytes);


#endif /* I2CTIVA_H_ */
