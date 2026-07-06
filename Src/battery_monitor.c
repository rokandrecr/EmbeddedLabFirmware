#include "battery_monitor.h"

/* ADC reference voltage in volts */
#define ADC_VREF                  3.3f

/* Maximum value for a 12-bit ADC */
#define ADC_MAX_VALUE             4095.0f

/* ADC timeout in milliseconds */
#define ADC_TIMEOUT_MS            10U

/*
 * ADC channels used for the two battery measurement nodes.
 *
 * NODE1: middle point between cell 1 and cell 2
 * NODE2: top of the 2-cell battery pack
 */
#define BATTERY_ADC_NODE1_CHANNEL ADC_CHANNEL_1
#define BATTERY_ADC_NODE2_CHANNEL ADC_CHANNEL_2

/*
 * Voltage divider gains.
 *
 * Real voltage = ADC pin voltage * divider gain
 *
 * Example:
 * If the divider is Rtop = 10k and Rbottom = 10k:
 * gain = (Rtop + Rbottom) / Rbottom = 2.0
 *
 * These values are placeholders. Change them later according to your real resistor values.
 */
#define NODE1_DIVIDER_GAIN        2.0f
#define NODE2_DIVIDER_GAIN        4.0f

/* Expected Li-ion cell voltage range */
#define CELL_MIN_VOLTAGE          2.5f
#define CELL_MAX_VOLTAGE          4.25f

/* Pointer to the ADC handler used by this module */
static ADC_HandleTypeDef *battery_adc = NULL;

/* Latest battery readings */
static BatteryReadings_t battery_readings = {0};

/*
 * Converts a raw 12-bit ADC value to the voltage seen directly at the ADC pin.
 */
static float BatteryMonitor_RawToAdcVoltage(uint32_t raw_value)
{
    return ((float)raw_value * ADC_VREF) / ADC_MAX_VALUE;
}

/*
 * Configures the selected ADC channel and reads one sample.
 */
static HAL_StatusTypeDef BatteryMonitor_ReadAdcChannel(uint32_t channel, uint32_t *raw_value)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    if ((battery_adc == NULL) || (raw_value == NULL))
    {
        return HAL_ERROR;
    }
    // Select the ADC channel and configure its rank and sampling time
    sConfig.Channel = channel;
    sConfig.Rank = 1;

    /*
     * A longer sampling time is safer when using voltage dividers,
     * especially if the resistor values are high.
     */
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;

    if (HAL_ADC_ConfigChannel(battery_adc, &sConfig) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_ADC_Start(battery_adc) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_ADC_PollForConversion(battery_adc, ADC_TIMEOUT_MS) != HAL_OK)
    {
        HAL_ADC_Stop(battery_adc);
        return HAL_TIMEOUT;
    }

    *raw_value = HAL_ADC_GetValue(battery_adc);

    HAL_ADC_Stop(battery_adc);

    return HAL_OK;
}

/*
 * Checks if both calculated cell voltages are inside a reasonable range.
 */
static uint8_t BatteryMonitor_ValidateReadings(float cell1_voltage, float cell2_voltage)
{
    if ((cell1_voltage < CELL_MIN_VOLTAGE) || (cell1_voltage > CELL_MAX_VOLTAGE))
    {
        return 0U;
    }

    if ((cell2_voltage < CELL_MIN_VOLTAGE) || (cell2_voltage > CELL_MAX_VOLTAGE))
    {
        return 0U;
    }

    return 1U;
}

void BatteryMonitor_Init(ADC_HandleTypeDef *hadc)
{
    battery_adc = hadc;

    battery_readings.raw_node1 = 0U;
    battery_readings.raw_node2 = 0U;

    battery_readings.adc_node1_voltage = 0.0f;
    battery_readings.adc_node2_voltage = 0.0f;

    battery_readings.node1_voltage = 0.0f;
    battery_readings.node2_voltage = 0.0f;

    battery_readings.cell1_voltage = 0.0f;
    battery_readings.cell2_voltage = 0.0f;

    battery_readings.valid = 0U;
}

void BatteryMonitor_Update(void)
{
    uint32_t raw_node1 = 0U;
    uint32_t raw_node2 = 0U;

    float adc_node1_voltage = 0.0f;
    float adc_node2_voltage = 0.0f;

    float node1_voltage = 0.0f;
    float node2_voltage = 0.0f;

    float cell1_voltage = 0.0f;
    float cell2_voltage = 0.0f;

    if (battery_adc == NULL)
    {
        battery_readings.valid = 0U;
        return;
    }
    // Read the adc value and store it in the raw_node variables.
    if (BatteryMonitor_ReadAdcChannel(BATTERY_ADC_NODE1_CHANNEL, &raw_node1) != HAL_OK)
    {
        battery_readings.valid = 0U;
        return;
    }

    if (BatteryMonitor_ReadAdcChannel(BATTERY_ADC_NODE2_CHANNEL, &raw_node2) != HAL_OK)
    {
        battery_readings.valid = 0U;
        return;
    }
    // Convert the raw ADC values to voltages at the ADC pins.
    adc_node1_voltage = BatteryMonitor_RawToAdcVoltage(raw_node1);
    adc_node2_voltage = BatteryMonitor_RawToAdcVoltage(raw_node2);

    node1_voltage = adc_node1_voltage * NODE1_DIVIDER_GAIN;
    node2_voltage = adc_node2_voltage * NODE2_DIVIDER_GAIN;

    /*
     * For a 2-cell series pack:
     *
     * Cell 1 is measured from GND to node 1.
     * Cell 2 is calculated as node 2 minus node 1.
     */
    cell1_voltage = node1_voltage;
    cell2_voltage = node2_voltage - node1_voltage;

    battery_readings.raw_node1 = raw_node1;
    battery_readings.raw_node2 = raw_node2;

    battery_readings.adc_node1_voltage = adc_node1_voltage;
    battery_readings.adc_node2_voltage = adc_node2_voltage;

    battery_readings.node1_voltage = node1_voltage;
    battery_readings.node2_voltage = node2_voltage;

    battery_readings.cell1_voltage = cell1_voltage;
    battery_readings.cell2_voltage = cell2_voltage;

    battery_readings.valid = BatteryMonitor_ValidateReadings(cell1_voltage, cell2_voltage);
}

BatteryReadings_t BatteryMonitor_GetReadings(void)
{
    return battery_readings;
}

uint32_t BatteryMonitor_GetRawValue(void)
{
    return battery_readings.raw_node1;
}

float BatteryMonitor_GetAdcVoltage(void)
{
    return battery_readings.adc_node1_voltage;
}