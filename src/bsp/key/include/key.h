#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

#include <stdbool.h>
#include <stdint.h>

#include <Servo.h>

#ifdef ESP32
// #include "../lib/include/ESP32_Servo.h"
#endif

typedef struct {
  uint8_t pin_num; // ESC가 연결된 핀 번호

  bool is_init;
  Servo *servo_h;
} bsp_key_handle_t;

bool bsp_key_init(bsp_key_handle_t *handler_p_);
bool bsp_key_set(bsp_key_handle_t *handler_p_, uint16_t degree_);

#endif