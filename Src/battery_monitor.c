/* Here lies the code for the battery monitor. 
The goal is to read the adc constantly*/

#include "battery_monitor.h"

/* ADC reference voltage in volts */
#define ADC_VREF        3.3f

/* Maximum value for a 12-bit ADC */
#define ADC_MAX_VALUE   4095.0f

/* Pointer to the ADC handler used by this module */
static ADC_HandleTypeDef *battery_adc = NULL;

/* Last raw ADC reading */
static uint32_t adc_raw_value = 0;

/* Last converted ADC voltage */
static float adc_voltage = 0.0f;

void BatteryMonitor_Init(ADC_HandleTypeDef *hadc)
{
    /* Store the ADC handler for later use */
    battery_adc = hadc;
}

void BatteryMonitor_Update(void)
{
    /* Check that the ADC handler was initialized */
    if (battery_adc == NULL)
    {
        return;
    }

    /* Start ADC conversion */
    HAL_ADC_Start(battery_adc);

    /* Wait until the ADC conversion is complete */
    if (HAL_ADC_PollForConversion(battery_adc, 10) == HAL_OK)
    {
        /* Read raw ADC value */
        adc_raw_value = HAL_ADC_GetValue(battery_adc);

        /* Convert raw ADC value to voltage */
        adc_voltage = ((float)adc_raw_value * ADC_VREF) / ADC_MAX_VALUE;
    }

    /* Stop ADC conversion */
    HAL_ADC_Stop(battery_adc);
}

uint32_t BatteryMonitor_GetRawValue(void)
{
    return adc_raw_value;
}

float BatteryMonitor_GetAdcVoltage(void)
{
    return adc_voltage;
}