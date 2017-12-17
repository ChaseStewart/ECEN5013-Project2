#include "sysUnderTest.h"

/*Function to convert the Tmp102 ADC values to Temperature*/
int16_t tempConversion(int16_t temp)
{
    temp = temp >>4;
    if(temp & 0x800) /*finding Negative values in 12-bit ADC result*/
    {
        temp = ~temp;   /*Finding Absolute Value by 2s Complement and multpily by -1*/
        temp++;
        return (int16_t)((float)temp*TEMP_SENSOR_RESOLUTION*(-1.0));
    }
    else    /*If Positive Values*/
    {
        return (int16_t)((float)temp*TEMP_SENSOR_RESOLUTION);
    }
}


float lightConversion(float CH0, float CH1)
{
    float intensity = 0;
    float selectFormula = 0;
    selectFormula = CH1/CH0;    /*Lux calculation depends on this ratio*/
    /*Formula Selection*/
    if(selectFormula <= 0.5)
    {
        intensity =  (0.0304*CH0)-(0.062*CH0*((CH1/CH0)*1.4));
    }
    else if(selectFormula <= 0.61)
    {
        intensity = (0.0224*CH0)-(0.031*CH1);
    }
    else if(selectFormula <= 0.8)
    {
        intensity =  (0.0128*CH0)-(0.0153*CH1);
    }
    else if(selectFormula <= 1.3)
    {
        intensity =  (0.00146*CH0)-(0.00112*CH1);
    }
    else
    {
        intensity = 0;
    }
    return intensity;
}

