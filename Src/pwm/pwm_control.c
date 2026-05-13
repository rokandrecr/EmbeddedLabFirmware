#include "pwm/pwm_control.h"

extern TIM_HandleTypeDef htim1;

/* If you wanna tickle the timer 4 that activates a led do this
#define PWM_TIMER_HANDLE   htim4
#define PWM_TIMER_CHANNEL  TIM_CHANNEL_1
*/
/* However we want to trigger a pwm on TIM1 where there is a normal output and complementary output*/
#define PWM_TIMER_HANDLE   htim1
#define PWM_TIMER_CHANNEL  TIM_CHANNEL_1

static uint8_t pwm_led_on = 0;

void PWM_Start(void)
{
    HAL_TIM_PWM_Start(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL);
    HAL_TIMEx_PWMN_Start(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL);
}

void PWM_Stop(void)
{
    HAL_TIM_PWM_Stop(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL);
}

void PWM_SetDutyRaw(uint32_t pulse)
{
    __HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL, pulse);
}

void PWM_SetDutyPercent(uint8_t percent)
{
    uint32_t arr;
    uint32_t pulse;

    if (percent > 100)
    {
        percent = 100;
    }

    arr = __HAL_TIM_GET_AUTORELOAD(&PWM_TIMER_HANDLE);

    pulse = ((arr + 1U) * percent) / 100U;

    __HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL, pulse);
}

void PWM_BlinkStep(void)
{
    if (pwm_led_on)
    {
        PWM_SetDutyPercent(0);
        pwm_led_on = 0;
    }
    else
    {
        PWM_SetDutyPercent(50);
        pwm_led_on = 1;
    }
}