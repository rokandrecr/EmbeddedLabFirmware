#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include "main.h"
#include <stdint.h>

/*
 * Initializes the battery monitor module with the ADC handler.
 */
void BatteryMonitor_Init(ADC_HandleTypeDef *hadc);

/*
 * Reads one ADC sample and updates the internal values.
 */
void BatteryMonitor_Update(void);

/*
 * Returns the last raw ADC value.
 */
uint32_t BatteryMonitor_GetRawValue(void);

/*
 * Returns the last converted ADC voltage.
 */
float BatteryMonitor_GetAdcVoltage(void);

#endif /* BATTERY_MONITOR_H */