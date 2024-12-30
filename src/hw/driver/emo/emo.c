#include <hardware/gpio.h>

#include "emo.h"

bool emo_init(void) {
  gpio_init(HWCONF_PIN_EMO_SWITCH);
  gpio_set_dir(HWCONF_PIN_EMO_SWITCH, GPIO_IN);

  return true;
}

bool emo_get_status(void) {
  return !gpio_get(HWCONF_PIN_EMO_SWITCH);
}
