#ifndef A12AC0BF_42FE_4F33_B58E_B44DA3D253A8_H_
#define A12AC0BF_42FE_4F33_B58E_B44DA3D253A8_H_

#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

bool actuator_task_init(void);

bool actuator_task_set_throttle(float percentage);
bool actuator_task_set_key(float degree);
bool actuator_task_set_rgbwled(uint8_t red, uint8_t green, uint8_t blue, uint8_t white);
bool actuator_task_set_power(bool power_target,
                             float key_min_degree,
                             float key_max_degree,
                             uint16_t key_pulse_0_degree,
                             uint16_t key_pulse_180_degree,
                             uint16_t thruster_pulse_0_percentage,
                             uint16_t thruster_pulse_100_percentage);
bool actuator_task_set_tone(uint16_t hz, uint16_t duration_ms);

#endif /* A12AC0BF_42FE_4F33_B58E_B44DA3D253A8_H_ */
