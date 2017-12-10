/****************************************************************
* file  : soilTIVA.h
* brief : This file implements the Soil Moisture Sensor functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef SOILTIVA_H_
#define SOILTIVA_H_

/* GPIO and Hardware */
#define SOIL_ADC            ADC0_BASE;
#define SOIL_SEQ_NO         3

#define SOIL_GPIO_PORT      GPIO_PORTE_BASE;
#define SOIL_GPIO_PIN       GPIO_PIN_3;


/****************************************************************
* name  : soilTask
* brief : This function is the start of the Soil Sensor Task which
*         handles soil moisture information
* param : Passed when creating the task
* return: None
*****************************************************************/
void soilTask(void *pvParameters);

/****************************************************************
* name  : soilSensorInit
* brief : Initialize the ADC and prepare to take readings
* param : None
* return: None
*****************************************************************/
void soilSensorInit(void);




#endif /* SOILTIVA_H_ */
