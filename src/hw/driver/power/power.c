#include <hardware/gpio.h>

#include "hal/time/time.h"

#include "power.h"

bool power_init(void) {
  gpio_init(HWCONF_PIN_POWER_MAIN_EN);
  gpio_init(HWCONF_PIN_POWER_SBC_EN);
  gpio_init(HWCONF_PIN_POWER_ACT_EN);
  gpio_init(HWCONF_PIN_SWITCH_POWER);

  gpio_set_dir(HWCONF_PIN_POWER_MAIN_EN, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_POWER_SBC_EN, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_POWER_ACT_EN, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_SWITCH_POWER, GPIO_IN);

  gpio_put(HWCONF_PIN_POWER_MAIN_EN, false);
  gpio_put(HWCONF_PIN_POWER_SBC_EN, false);
  gpio_put(HWCONF_PIN_POWER_ACT_EN, false);

  return true;
}

void power_set_main(bool s) {
  gpio_put(HWCONF_PIN_POWER_MAIN_EN, s);

  // block until power off
  while (!s) {
    time_block_ms(1000);
  }
}

void power_set_sbc(bool s) {
  gpio_put(HWCONF_PIN_POWER_SBC_EN, s);
}

bool power_get_sbc(void) {
  return gpio_get(HWCONF_PIN_POWER_SBC_EN);
}

void power_set_act(bool s) {
  gpio_put(HWCONF_PIN_POWER_ACT_EN, s);
}

bool power_get_act(void) {
  return gpio_get(HWCONF_PIN_POWER_ACT_EN);
}

bool power_get_button(void) {
  return !gpio_get(HWCONF_PIN_SWITCH_POWER);
}
