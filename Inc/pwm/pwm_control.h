#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include "stm32f4xx_hal.h"

/*
 * PWM control module
 * This module starts/stops a PWM channel and changes its duty cycle.
 */

void PWM_Start(void);
void PWM_Stop(void);
void PWM_SetDutyRaw(uint32_t pulse);
void PWM_SetDutyPercent(uint8_t percent);
void PWM_BlinkStep(void);

#endif /* PWM_CONTROL_H */