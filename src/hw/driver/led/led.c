#include <hardware/gpio.h>

#include "led.h"

bool led_init(void) {
  gpio_init(HWCONF_PIN_LED_STATUS);
  gpio_init(HWCONF_PIN_LED_USER_1);
  gpio_init(HWCONF_PIN_LED_USER_2);
  gpio_init(HWCONF_PIN_LED_FAULT);

  gpio_set_dir(HWCONF_PIN_LED_STATUS, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_LED_USER_1, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_LED_USER_2, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_LED_FAULT, GPIO_OUT);

  gpio_put(HWCONF_PIN_LED_STATUS, false);
  gpio_put(HWCONF_PIN_LED_USER_1, false);
  gpio_put(HWCONF_PIN_LED_USER_2, false);
  gpio_put(HWCONF_PIN_LED_FAULT, false);
}

void led_set_s(bool s) {
  gpio_put(HWCONF_PIN_LED_STATUS, s);
}

void led_set_1(bool s) {
  gpio_put(HWCONF_PIN_LED_USER_1, s);
}

void led_set_2(bool s) {
  gpio_put(HWCONF_PIN_LED_USER_2, s);
}

void led_set_f(bool s) {
  gpio_put(HWCONF_PIN_LED_FAULT, s);
}
