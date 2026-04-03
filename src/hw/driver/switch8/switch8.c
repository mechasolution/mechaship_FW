#include <zephyr/drivers/gpio.h>

#include "hal/time/time.h"
#include "hwconf.h"
#include "switch8.h"

static const struct gpio_dt_spec s_latch = HWCONF_SHIFT_LATCH_SPEC;
static const struct gpio_dt_spec s_clk = HWCONF_SHIFT_CLK_SPEC;
static const struct gpio_dt_spec s_data = HWCONF_SHIFT_DATA_SPEC;

bool switch8_init(void) {
  if (!gpio_is_ready_dt(&s_latch) || !gpio_is_ready_dt(&s_clk) || !gpio_is_ready_dt(&s_data)) {
    return false;
  }

  if (gpio_pin_configure_dt(&s_latch, GPIO_OUTPUT_ACTIVE) < 0) {
    return false;
  }
  if (gpio_pin_configure_dt(&s_clk, GPIO_OUTPUT_INACTIVE) < 0) {
    return false;
  }
  if (gpio_pin_configure_dt(&s_data, GPIO_INPUT) < 0) {
    return false;
  }

  return true;
}

uint8_t switch8_get_sum(void) {
  gpio_pin_set_dt(&s_latch, 0);
  time_block_ns(4);
  gpio_pin_set_dt(&s_latch, 1);
  time_block_ns(4);

  uint8_t ret = 0;

  for (int8_t i = 7; i >= 0; i--) {
    ret += (uint8_t)(gpio_pin_get_dt(&s_data) > 0 ? 1 : 0) << i;

    gpio_pin_set_dt(&s_clk, 1);
    time_block_ns(4);
    gpio_pin_set_dt(&s_clk, 0);
    time_block_ns(4);
  }

  return ret;
}
