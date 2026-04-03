#include <zephyr/drivers/gpio.h>

#include "hwconf.h"
#include "emo.h"

static const struct gpio_dt_spec s_emo_switch = HWCONF_EMO_SWITCH_SPEC;

bool emo_init(void) {
  if (!gpio_is_ready_dt(&s_emo_switch)) {
    return false;
  }

  return gpio_pin_configure_dt(&s_emo_switch, GPIO_INPUT) == 0;
}

bool emo_get_status(void) {
  return gpio_pin_get_dt(&s_emo_switch) > 0;
}
