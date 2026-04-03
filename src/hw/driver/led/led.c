#include <zephyr/drivers/gpio.h>
#include <errno.h>

#include "hwconf.h"
#include "led.h"

static const struct gpio_dt_spec s_led_status = HWCONF_LED_STATUS_SPEC;
static const struct gpio_dt_spec s_led_ros = HWCONF_LED_ROS_MODE_SPEC;
static const struct gpio_dt_spec s_led_rc = HWCONF_LED_RC_MODE_SPEC;
static const struct gpio_dt_spec s_led_fault = HWCONF_LED_FAULT_SPEC;

static int s_config_output(const struct gpio_dt_spec *spec) {
  if (!gpio_is_ready_dt(spec)) {
    return -ENODEV;
  }

  return gpio_pin_configure_dt(spec, GPIO_OUTPUT_INACTIVE);
}

bool led_init(void) {
  if (s_config_output(&s_led_status) < 0) {
    return false;
  }
  if (s_config_output(&s_led_ros) < 0) {
    return false;
  }
  if (s_config_output(&s_led_rc) < 0) {
    return false;
  }
  if (s_config_output(&s_led_fault) < 0) {
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
