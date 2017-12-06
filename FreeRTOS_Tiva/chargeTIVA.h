/****************************************************************
* file  : chargeTIVA.h
* brief : This file implements the functionalities to monitor battery charge
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef CHARGETIVA_H_
#define CHARGETIVA_H_

/* Hardware */
#define CHARGE_SLAVE_ADDRESS    0x6C       /*Slave Address*/
#define I2C_DEVICE              I2C0_BASE

/* Registers */
#define REG_CONFIG_ALERT_PCT    0x0C        /* Config reg */
#define REG_CONFIG_RESET        0xFE        /* Reset reg */
#define REG_READ_REGISTER       0x02        /* Reg for reading charge percent*/
#define REG_VERSION             0x08        /* Reg for version */

/*Configuration values */
#define ALERT_HIGH_BYTE         0x97 /* high and low are equiv to 32% */
#define ALERT_LOW_BYTE          0x00
#define RESET_HIGH_BYTE         0x54 /* high and low are equiv to 32% */
#define RESET_LOW_BYTE          0x00
#define WRITE_FLAG  false
#define READ_FLAG   true


/****************************************************************
* name  : chargeTask
* brief : This function is the start of the Fuel Gauge sensor Handling
* param : Passed when creating the task
* return: None
*****************************************************************/
void chargeTask(void *pvParameters);


/****************************************************************
* name  : chargeTask
* brief : Initialize the charge sensor registers- TODO move gpio/i2c stuff to main.
* param : None
* return: None
*****************************************************************/
void chargeSensorInit(void);

/****************************************************************
* name  : writeNBytes
* brief : use I2C to write 1 byte to provided register
* param : uint8_t regAddr - the I2C register into which to write
* param : uint8_t data    - the data that should be written
* return: int8_t status   - 0 is success else failure
*****************************************************************/
int8_t chgwriteData(uint8_t regAddr, uint8_t data);


/****************************************************************
* name  : writeNBytes
* brief : use I2C to write N bytes to provided register
* param : uint8_t regAddr - the I2C register into which to write
* param : uint8_t *data   - a pointer to the data that should be written
* param : uint8_t length  - number of bytes to write to I2C register
* return: int8_t status   - 0 is success else failure
*****************************************************************/
int8_t chgwriteNBytes(uint8_t regAddr, uint8_t *data, uint8_t length);

/****************************************************************
* name  : readData
* brief : This function reads 1 or 2 bytes from the provided register
* param : uint8_t regAddr - the I2C register from which to read
* param : uint8_t *data   - the data pointer which will contain the read bytes
* param : uint8_t bytes   - number of bytes to read
* return: int8_t status   - 0 is success else failure
*****************************************************************/
int8_t chgreadData(uint8_t regAddr, uint8_t *data, uint8_t bytes);


/****************************************************************
* name  : getChargeData
* brief : This function uses the functions above to get the percent of charge
* param : uint16_t *chargeAmt  - the data pointer which will contain the pct of charge from 0-100
* return: int8_t status       - 0 is success else failure
*****************************************************************/
int8_t getChargeData(uint16_t *charge_amt);



#endif /* CHARGETIVA_H_ */
