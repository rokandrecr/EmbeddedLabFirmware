#ifndef BALANCER_CONTROLLER_H
#define BALANCER_CONTROLLER_H

#include <stdint.h>

/*
 * Balancer controller module
 *
 * Current version uses mocked cell voltages because the voltage sensing circuit
 * is not available yet. Later, the mock function can be replaced by ADC reads.
 */

typedef struct
{
    float cell1_voltage;
    float cell2_voltage;
    float voltage_difference;
    uint8_t balancing_active;
} BalancerStatus_t;

void BalancerController_Init(void);
void BalancerController_Update(void);
BalancerStatus_t BalancerController_GetStatus(void);

#endif /* BALANCER_CONTROLLER_H */
