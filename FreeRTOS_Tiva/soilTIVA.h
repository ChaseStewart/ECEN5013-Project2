/****************************************************************
* file  : soilTIVA.h
* brief : This file implements the Soil Moisture Sensor functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef SOILTIVA_H_
#define SOILTIVA_H_

/****************************************************************
* name  : soilTask
* brief : This function is the start of the Soil Sensor Task which
*         handles soil moisture information
* param : Passed when creating the task
* return: None
*****************************************************************/
void soilTask(void *pvParameters);

#endif /* SOILTIVA_H_ */
