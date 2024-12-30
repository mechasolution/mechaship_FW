#include <hardware/gpio.h>

#include "hal/time/time.h"

#include "power.h"

static uint32_t s_power_button_last_change_timestamp = 0;
static power_button_callback_t cb = NULL;

void power_button_irq_handler(uint gpio, uint32_t events) {
  s_power_button_last_change_timestamp = time_get_millis();

  if (cb != NULL) {
    cb();
  }
}

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

  gpio_set_irq_enabled_with_callback(
      HWCONF_PIN_SWITCH_POWER,
      GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
      true,
      power_button_irq_handler);

  return true;
}

void power_set_button_callback(power_button_callback_t callback) {
  if (callback == NULL) {
    return;
  }

  cb = callback;
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

power_button_status_t *power_get_button_status(power_button_status_t *data) {
  data->status = !gpio_get(HWCONF_PIN_SWITCH_POWER);
  data->last_change_timestamp = s_power_button_last_change_timestamp;

  return data;
}

bool power_get_button(void) {
  return !gpio_get(HWCONF_PIN_SWITCH_POWER);
}
