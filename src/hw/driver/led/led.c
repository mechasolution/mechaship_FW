#include <hardware/gpio.h>

#include "led.h"

bool led_init(void) {
  gpio_init(HWCONF_PIN_LED_STATUS);
  gpio_init(HWCONF_PIN_LED_ROS_MODE);
  gpio_init(HWCONF_PIN_LED_RC_MODE);
  gpio_init(HWCONF_PIN_LED_FAULT);

  gpio_set_dir(HWCONF_PIN_LED_STATUS, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_LED_ROS_MODE, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_LED_RC_MODE, GPIO_OUT);
  gpio_set_dir(HWCONF_PIN_LED_FAULT, GPIO_OUT);

  gpio_put(HWCONF_PIN_LED_STATUS, false);
  gpio_put(HWCONF_PIN_LED_ROS_MODE, false);
  gpio_put(HWCONF_PIN_LED_RC_MODE, false);
  gpio_put(HWCONF_PIN_LED_FAULT, false);
}
void led_test(bool st) {
  if (st) {
    gpio_put(HWCONF_PIN_LED_ROS_MODE, true);
    gpio_put(HWCONF_PIN_LED_RC_MODE, true);
    gpio_put(HWCONF_PIN_LED_STATUS, true);
    gpio_put(HWCONF_PIN_LED_FAULT, true);
  } else {
    gpio_put(HWCONF_PIN_LED_ROS_MODE, false);
    gpio_put(HWCONF_PIN_LED_RC_MODE, false);
    gpio_put(HWCONF_PIN_LED_STATUS, false);
    gpio_put(HWCONF_PIN_LED_FAULT, false);
  }
}

void led_set_s(bool s) {
  gpio_put(HWCONF_PIN_LED_STATUS, s);
}

void led_set_rc_mode(bool s) {
  gpio_put(HWCONF_PIN_LED_RC_MODE, s);
}

void led_set_ros_mode(bool s) {
  gpio_put(HWCONF_PIN_LED_ROS_MODE, s);
}

void led_set_f(bool s) {
  gpio_put(HWCONF_PIN_LED_FAULT, s);
}
