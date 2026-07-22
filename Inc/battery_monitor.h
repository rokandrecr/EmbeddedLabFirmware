#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include "main.h"
#include <stdint.h>

/*
 * Stores the complete voltage information for a 2-cell battery pack.
 *
 * node1_voltage = voltage from pack GND to the middle point between cell 1 and cell 2
 * node2_voltage = voltage from pack GND to the top of cell 2
 *
 * cell1_voltage = node1_voltage
 * cell2_voltage = node2_voltage - node1_voltage
 */
typedef struct
{
    uint32_t raw_node1;
    uint32_t raw_node2;

    float adc_node1_voltage;
    float adc_node2_voltage;

    float node1_voltage;
    float node2_voltage;

    float cell1_voltage;
    float cell2_voltage;

    uint8_t valid;
} BatteryReadings_t;

/*
 * Initializes the battery monitor module with the ADC handler.
 */
void BatteryMonitor_Init(ADC_HandleTypeDef *hadc);

/*
 * Reads both ADC channels and updates the internal battery readings.
 */
void BatteryMonitor_Update(void);

/*
 * Returns the complete latest battery readings.
 */
BatteryReadings_t BatteryMonitor_GetReadings(void);

/*
 * Compatibility function with the previous version.
 * Returns the last raw value of node 1.
 */
uint32_t BatteryMonitor_GetRawValue(void);

/*
 * Compatibility function with the previous version.
 * Returns the last ADC voltage of node 1.
 */
float BatteryMonitor_GetAdcVoltage(void);

#endif /* BATTERY_MONITOR_H */