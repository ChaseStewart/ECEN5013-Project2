/****************************************************************
* file  : soilTIVA.c
* brief : This file implements the Soil Moisture Sensor functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#include "common.h"
#include "soilTIVA.h"

#include "inc/hw_adc.h"
#include "inc/hw_memmap.h"

#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

/*Global variable for Queue Handles in main*/
extern QueueHandle_t mainQueue;
extern QueueHandle_t tempQueue;
extern QueueHandle_t lightQueue;
extern QueueHandle_t socketQueue;
extern QueueHandle_t soilQueue;
extern QueueHandle_t chargeQueue;
uint32_t data;

/*
void ISR_ADC_Read (void)
{

    ADCIntClear(SOIL_ADC, SOIL_SEQ_NO);
    while(!ADCIntStatus(SOIL_ADC, SOIL_SEQ_NO, false));

    ADCSequenceDataGet(SOIL_ADC, SOIL_SEQ_NO, &data);

}
*/

void soilTask(void *pvParameters)
{
    uint32_t humidData;

    UARTprintf("\r\nInitializing Soil Task");

    //soilSensorInit();

    while(1)
    {
        /* trigger an ADC read*/
        ADCProcessorTrigger(ADC0_BASE, SOIL_SEQ_NO);

        /* wait for ADC to read and clear int */
        while(!ADCIntStatus(ADC0_BASE, SOIL_SEQ_NO, false))
        {
        }
        ADCIntClear(ADC0_BASE, 3);

        /* get results */
        ADCSequenceDataGet(ADC0_BASE, SOIL_SEQ_NO, &humidData);
        UARTprintf("\r\nMain Task result: %d",humidData);

        /* delay for 1/2 of a second */
        SysCtlDelay(SYSTEM_CLOCK/3/2);

    }

    vTaskDelete(NULL);  /*Deletes Current task and frees up memory*/
}


void soilSensorInit(void)
{
    /* the soil sensor's init is implemented in main's myADCInit */
}
