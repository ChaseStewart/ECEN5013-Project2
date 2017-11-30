/****************************************************************
* file  : chargeTIVA.h
* brief : This file implements the functionalities to monitor battery charge
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef CHARGETIVA_H_
#define CHARGETIVA_H_

/****************************************************************
* name  : chargeTask
* brief : This function is the start of the Fuel Gauge sensor Handling
* param : Passed when creating the task
* return: None
*****************************************************************/
void chargeTask(void *pvParameters);

#endif /* CHARGETIVA_H_ */
