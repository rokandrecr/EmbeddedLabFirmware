#include "pwm/pwm_control.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;

/* If you wanna tickle the timer 4 that activates a led do this
#define PWM_TIMER_HANDLE   htim4
#define PWM_TIMER_CHANNEL  TIM_CHANNEL_1
*/
/* However we want to trigger a pwm on TIM1 where there is a normal output and complementary output*/
#define PWM_TIMER_HANDLE   htim1
#define PWM_TIMER_CHANNEL  TIM_CHANNEL_1
#define PWM_TIMER_HANDLE2   htim4
#define PWM_TIMER_CHANNEL2  TIM_CHANNEL_1

static uint8_t pwm_led_on = 0;

void PWM_Start(void)
{   
    /* Start PWM on both timers */
    HAL_TIM_PWM_Start(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL);
    HAL_TIM_PWM_Start(&PWM_TIMER_HANDLE2, PWM_TIMER_CHANNEL2);
    /* Start complementary PWM on the first timer */
    HAL_TIMEx_PWMN_Start(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL);

}

void PWM_Stop(void)
{   
    /* Stop PWM on both timers */
    HAL_TIM_PWM_Stop(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL);
    HAL_TIM_PWM_Stop(&PWM_TIMER_HANDLE2, PWM_TIMER_CHANNEL2);

}

void PWM_SetDutyRaw(uint32_t pulse)
{
    __HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL, pulse);
}

void PWM_SetDutyPercent(uint8_t percent)
{
    uint32_t arr_tim1;
    uint32_t arr_tim4;
    uint32_t pulse_tim1;
    uint32_t pulse_tim4;

    if (percent > 100)
    {
        percent = 100;
    }

    // TIM1 duty
    arr_tim1 = __HAL_TIM_GET_AUTORELOAD(&htim1);
    pulse_tim1 = ((arr_tim1 + 1U) * percent) / 100U;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse_tim1);

    // TIM4 duty
    arr_tim4 = __HAL_TIM_GET_AUTORELOAD(&htim4);
    pulse_tim4 = ((arr_tim4 + 1U) * percent) / 100U;
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse_tim4);
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