#include "hardware/gpio.h"

#include "hal/time/time.h"

#include "switch8.h"

bool switch8_init(void) {
  gpio_init(HWCONF_PIN_SHIFT_LATCH);
  gpio_init(HWCONF_PIN_SHIFT_CLK);
  gpio_init(HWCONF_PIN_SHIFT_DATA);

  gpio_set_dir(HWCONF_PIN_SHIFT_LATCH, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_SHIFT_CLK, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_SHIFT_DATA, GPIO_IN);

  gpio_put(HWCONF_PIN_SHIFT_LATCH, true);
  gpio_put(HWCONF_PIN_SHIFT_CLK, false);

  return true;
}

uint8_t switch8_get_sum(void) {
  gpio_put(HWCONF_PIN_SHIFT_LATCH, false);
  time_block_ns(4);
  gpio_put(HWCONF_PIN_SHIFT_LATCH, true);
  time_block_ns(4);

  uint8_t ret = 0;

  for (int8_t i = 7; i >= 0; i--) {
    ret += gpio_get(HWCONF_PIN_SHIFT_DATA) << i;

    gpio_put(HWCONF_PIN_SHIFT_CLK, true);
    time_block_ns(4);
    gpio_put(HWCONF_PIN_SHIFT_CLK, false);
    time_block_ns(4);
  }

  return ret;
}