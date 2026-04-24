#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "power.h"

static const struct gpio_dt_spec s_main_en = GPIO_DT_SPEC_GET(DT_NODELABEL(power_ctrl), main_en_gpios);
static const struct gpio_dt_spec s_sbc_en = GPIO_DT_SPEC_GET(DT_NODELABEL(power_ctrl), sbc_en_gpios);
static const struct gpio_dt_spec s_act_en = GPIO_DT_SPEC_GET(DT_NODELABEL(power_ctrl), act_en_gpios);
static const struct gpio_dt_spec s_switch_power = GPIO_DT_SPEC_GET(DT_NODELABEL(power_ctrl), switch_gpios);

bool power_init(void) {
  if (!gpio_is_ready_dt(&s_main_en) ||
      !gpio_is_ready_dt(&s_sbc_en) ||
      !gpio_is_ready_dt(&s_act_en) ||
      !gpio_is_ready_dt(&s_switch_power)) {
    return false;
  }

  if (gpio_pin_configure_dt(&s_main_en, GPIO_OUTPUT_INACTIVE) < 0) {
    return false;
  }

  if (gpio_pin_configure_dt(&s_sbc_en, GPIO_OUTPUT_INACTIVE) < 0) {
    return false;
  }

  if (gpio_pin_configure_dt(&s_act_en, GPIO_OUTPUT_INACTIVE) < 0) {
    return false;
  }

  if (gpio_pin_configure_dt(&s_switch_power, GPIO_INPUT) < 0) {
    return false;
  }

  return true;
}

void power_set_main(bool s) {
  gpio_pin_set_dt(&s_main_en, s ? 1 : 0);

  if (s == false) {
    k_sleep(K_FOREVER);
  }
}

void power_set_sbc(bool s) {
  gpio_pin_set_dt(&s_sbc_en, s ? 1 : 0);
}

bool power_get_sbc(void) {
  return gpio_pin_get_dt(&s_sbc_en) > 0;
}

void power_set_act(bool s) {
  gpio_pin_set_dt(&s_act_en, s ? 1 : 0);
}

bool power_get_act(void) {
  return gpio_pin_get_dt(&s_act_en) > 0;
}

bool power_get_button(void) {
  return gpio_pin_get_dt(&s_switch_power) > 0;
}
