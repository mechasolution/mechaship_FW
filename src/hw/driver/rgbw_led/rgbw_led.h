#ifndef BF84AEC7_D421_41C1_B0A9_C9F6D7F852F8_H_
#define BF84AEC7_D421_41C1_B0A9_C9F6D7F852F8_H_

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t rgbw_color_data_t;

bool rgbw_led_init(void);
void rgbw_led_test(bool st);
void rgbw_led_set_pixels(rgbw_color_data_t color);
void rgbw_led_set_onboard_pixel(rgbw_color_data_t color);

inline rgbw_color_data_t rgbw_led_get_color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return ((uint32_t)(b) << 8) |
         ((uint32_t)(r) << 16) |
         ((uint32_t)(g) << 24) |
         (uint32_t)(w);
}

inline void rgbw_led_clear_all(void) {
  rgbw_led_set_pixels(0);
}

#endif /* BF84AEC7_D421_41C1_B0A9_C9F6D7F852F8_H_ */
