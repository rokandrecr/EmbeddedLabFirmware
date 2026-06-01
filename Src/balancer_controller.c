#include "balancer_controller.h"
#include "pwm/pwm_control.h"
#include "main.h"

/* --------------------------------------------------------------------------
 * Mock configuration
 * --------------------------------------------------------------------------
 * These values simulate the voltage readings from two battery cells.
 * Change these values to test different balancing scenarios.
 */
#define MOCK_CELL1_VOLTAGE_V       4.10f
#define MOCK_CELL2_VOLTAGE_V       3.45f

/* Balancing threshold in volts.
 * If the absolute voltage difference is greater than this value,
 * balancing is enabled.
 */
#define BALANCE_THRESHOLD_V        0.50f

/* PWM duty cycle used when balancing is active. */
#define BALANCE_PWM_DUTY_PERCENT   50U

// Balancer Status is the structure where all the values needed for the balancing are placed
static BalancerStatus_t balancer_status = {0};
static uint8_t pwm_is_running = 0U;

// Only work with abs values
static float absolute_float(float value)
{
    if (value < 0.0f)
    {
        return -value;
    }

    return value;
}

/* Later this function can be replaced by real ADC voltage readings. */
static void BalancerController_ReadMockVoltages(float *cell1, float *cell2)
{
    if ((cell1 == NULL) || (cell2 == NULL))
    {
        return;
    }

    *cell1 = MOCK_CELL1_VOLTAGE_V;
    *cell2 = MOCK_CELL2_VOLTAGE_V;
}

/* Enable balancing by starting PWM with the defined duty cycle. */
static void BalancerController_EnableBalancing(void)
{
    if (pwm_is_running == 0U)
    {
        PWM_Start();
        pwm_is_running = 1U;
    }

    PWM_SetDutyPercent(BALANCE_PWM_DUTY_PERCENT);
    balancer_status.balancing_active = 1U;

    /* Optional visual feedback: red LED ON when balancing is active. */
    HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
}

/* Disable balancing by stopping PWM. */
static void BalancerController_DisableBalancing(void)
{
    PWM_SetDutyPercent(0U);
    PWM_Stop();
    pwm_is_running = 0U;
    balancer_status.balancing_active = 0U;

    /* Optional visual feedback: red LED OFF when balancing is inactive. */
    HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_RESET);
}

void BalancerController_Init(void)
{
    balancer_status.cell1_voltage = 0.0f;
    balancer_status.cell2_voltage = 0.0f;
    balancer_status.voltage_difference = 0.0f;
    balancer_status.balancing_active = 0U;
    pwm_is_running = 0U;

    BalancerController_DisableBalancing();
}

void BalancerController_Update(void)
{
    float cell1 = 0.0f;
    float cell2 = 0.0f;
    float difference = 0.0f;

    BalancerController_ReadMockVoltages(&cell1, &cell2);

    difference = absolute_float(cell1 - cell2);

    balancer_status.cell1_voltage = cell1;
    balancer_status.cell2_voltage = cell2;
    balancer_status.voltage_difference = difference;

    if (difference > BALANCE_THRESHOLD_V)
    {
        BalancerController_EnableBalancing();
    }
    else
    {
        BalancerController_DisableBalancing();
    }
}

BalancerStatus_t BalancerController_GetStatus(void)
{
    return balancer_status;
}
