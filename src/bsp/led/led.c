#include <errno.h>

#include <zephyr/drivers/gpio.h>

#include "led.h"

static const struct gpio_dt_spec s_led_status = GPIO_DT_SPEC_GET(DT_NODELABEL(status_led), gpios);
static const struct gpio_dt_spec s_led_ros = GPIO_DT_SPEC_GET(DT_NODELABEL(ros_mode_led), gpios);
static const struct gpio_dt_spec s_led_rc = GPIO_DT_SPEC_GET(DT_NODELABEL(rc_mode_led), gpios);
static const struct gpio_dt_spec s_led_fault = GPIO_DT_SPEC_GET(DT_NODELABEL(fault_led), gpios);

bool led_init(void) {
  if (!gpio_is_ready_dt(&s_led_status) ||
      !gpio_is_ready_dt(&s_led_ros) ||
      !gpio_is_ready_dt(&s_led_rc) ||
      !gpio_is_ready_dt(&s_led_fault)) {
    return false;
  }

  if (gpio_pin_configure_dt(&s_led_status, GPIO_OUTPUT_INACTIVE) < 0) {
    return false;
  }
  if (gpio_pin_configure_dt(&s_led_ros, GPIO_OUTPUT_INACTIVE) < 0) {
    return false;
  }
  if (gpio_pin_configure_dt(&s_led_rc, GPIO_OUTPUT_INACTIVE) < 0) {
    return false;
  }
  if (gpio_pin_configure_dt(&s_led_fault, GPIO_OUTPUT_INACTIVE) < 0) {
    return false;
  }

  return true;
}

void led_test(bool st) {
  gpio_pin_set_dt(&s_led_ros, st ? 1 : 0);
  gpio_pin_set_dt(&s_led_rc, st ? 1 : 0);
  gpio_pin_set_dt(&s_led_status, st ? 1 : 0);
  gpio_pin_set_dt(&s_led_fault, st ? 1 : 0);
}

void led_set_s(bool s) {
  gpio_pin_set_dt(&s_led_status, s ? 1 : 0);
}

void led_set_rc_mode(bool s) {
  gpio_pin_set_dt(&s_led_rc, s ? 1 : 0);
}

void led_set_ros_mode(bool s) {
  gpio_pin_set_dt(&s_led_ros, s ? 1 : 0);
}

void led_set_f(bool s) {
  gpio_pin_set_dt(&s_led_fault, s ? 1 : 0);
}
