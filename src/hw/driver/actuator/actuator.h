#ifndef F8AFB3C0_15D0_4867_BE14_6C2A8D9A16F6_H_
#define F8AFB3C0_15D0_4867_BE14_6C2A8D9A16F6_H_

#include <stdbool.h>
#include <stdint.h>

bool actuator_init(void);
void actuator_pwm_off(void);

void actuator_set_key_info(uint16_t pulse_0, uint16_t pulse_180, float min_degree, float max_degree);
void actuator_set_thruster_info(uint16_t pulse_0, uint16_t pulse_100);

void actuator_set_key_degree(float degree);
void actuator_set_key_pulse(uint16_t us);
float actuator_get_key_degree(void);
uint16_t actuator_get_key_pulse(void);

void actuator_set_thruster_percentage(float percentage);
void actuator_set_thruster_pulse(uint16_t us);
float actuator_get_thruster_percentage(void);
uint16_t actuator_get_thruster_pulse(void);

#endif /* F8AFB3C0_15D0_4867_BE14_6C2A8D9A16F6_H_ */
