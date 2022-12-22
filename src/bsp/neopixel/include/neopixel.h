#ifndef __BSP_NEOPIXEL_H__
#define __BSP_NEOPIXEL_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <Adafruit_NeoPixel.h>

typedef struct {
  uint8_t pin_num;  // 네오픽셀이 연결된 핀 번호
  size_t pixel_cnt; // 네오픽셀 수

  bool is_init;
  Adafruit_NeoPixel *pixel_h;
} bsp_neopixel_handler_t;

bool bsp_neopixel_init(bsp_neopixel_handler_t *handler_p_);
bool bsp_neopixel_set(bsp_neopixel_handler_t *handler_p_, uint8_t R_, uint8_t G_, uint8_t B_, uint8_t W_);

#endif