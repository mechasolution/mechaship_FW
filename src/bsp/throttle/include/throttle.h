#ifndef __BSP_THROTTLE_H__
#define __BSP_THROTTLE_H__

#include <stdbool.h>
#include <stdint.h>

#include <Servo.h>

#ifdef ESP32
// #include "../lib/include/ESP32_Servo.h"
#endif

typedef struct {
  uint8_t pin_num;         // ESC가 연결된 핀 번호
  int min_pwr_pulse_width; // 모터가 멈출 때 펄스폭
  int max_pwr_pulse_width; // 최대속도 펄스폭

  bool is_init;
  bool is_emo;
  Servo *servo_h;
} bsp_throttle_handle_t;

bool bsp_throttle_init(bsp_throttle_handle_t *handler_p_);
bool bsp_throttle_set_by_percentage(bsp_throttle_handle_t *handler_p_, uint8_t percentage_);
bool bsp_throttle_set_by_pulse_width(bsp_throttle_handle_t *handler_p_, uint16_t pulse_width_);
void bsp_throttle_set_emo(bsp_throttle_handle_t *handler_p_);
void bsp_throttle_reset_emo(bsp_throttle_handle_t *handler_p_);

#endif