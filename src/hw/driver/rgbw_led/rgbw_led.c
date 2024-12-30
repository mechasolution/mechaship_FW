#include "hardware/pio.h"

#include "sk6812.pio.h"

#include "hal/time/time.h"

#include "rgbw_led.h"

#define PIO_CHANNEL pio0

bool rgbw_led_init(void) {
  int sm = pio_claim_unused_sm(PIO_CHANNEL, true);

  uint offset = pio_add_program(PIO_CHANNEL, &sk6812_program);
  sk6812_program_init(PIO_CHANNEL, sm, offset, HWCONF_PIN_RGBW_LED, 800000, true);

  rgbw_led_clear_all();

  time_block_ms(1);

  return true;
}

void rgbw_led_set_pixels(rgbw_color_data_t color) {
  for (uint8_t i = 0; i < HWCONF_RGBW_LED_PIXEL_CNT; i++) {
    pio_sm_put_blocking(PIO_CHANNEL, 0, color);
  }
}

void rgbw_led_set_onboard_pixel(rgbw_color_data_t color) {
  pio_sm_put_blocking(PIO_CHANNEL, 0, color);
}
