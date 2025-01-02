#ifndef A12AC0BF_42FE_4F33_B58E_B44DA3D253A8_H_
#define A12AC0BF_42FE_4F33_B58E_B44DA3D253A8_H_

#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

typedef struct {
  enum {
    ACTUATOR_TASK_COMMAND_NONE = 0x00,

    ACTUATOR_TASK_COMMAND_THROTTLE,
    ACTUATOR_TASK_COMMAND_KEY,
    ACTUATOR_TASK_COMMAND_LED_1,
    ACTUATOR_TASK_COMMAND_LED_2,
    ACTUATOR_TASK_COMMAND_LED_S,
    ACTUATOR_TASK_COMMAND_LED_F,
    ACTUATOR_TASK_COMMAND_RGBWLED,
    ACTUATOR_TASK_COMMAND_POWER,
    ACTUATOR_TASK_COMMAND_TONE,
  } command;
  union {
    struct { // ACTUATOR_TASK_COMMAND_THROTTLE
      float percentage;
    } throttle;

    struct { // ACTUATOR_TASK_COMMAND_KEY
      float degree;
    } key;

    struct { // ACTUATOR_TASK_COMMAND_LED_1
      bool value;
    } led_1;

    struct { // ACTUATOR_TASK_COMMAND_LED_2
      bool value;
    } led_2;

    struct { // ACTUATOR_TASK_COMMAND_LED_S
      bool value;
    } led_s;

    struct { // ACTUATOR_TASK_COMMAND_LED_F
      bool value;
    } led_f;

    struct { // ACTUATOR_TASK_COMMAND_RGBWLED
      uint8_t red;
      uint8_t green;
      uint8_t blue;
      uint8_t white;
    } rgbwled;

    struct { // ACTUATOR_TASK_COMMAND_POWER
      bool power_target;

      float key_min_degree; // 키 최소 각도
      float key_max_degree; // 키 최대 각도

      uint16_t key_pulse_0_degree;   // 키 최소 각도 펄스
      uint16_t key_pulse_180_degree; // 키 최대 각도 펄스

      uint16_t thruster_pulse_0_percentage;   // ESC 중립 펄스
      uint16_t thruster_pulse_100_percentage; // ESC 최대속도 펄스
    } power;

    struct { // ACTUATOR_TASK_COMMAND_TONE
      uint16_t hz;
      uint16_t duration_ms;
    } tone;
  } data;

  // TODO: response
} actuator_task_queue_data_t;

extern QueueHandle_t actuator_task_queue_hd;
extern TaskHandle_t actuator_task_hd;

#endif /* A12AC0BF_42FE_4F33_B58E_B44DA3D253A8_H_ */
