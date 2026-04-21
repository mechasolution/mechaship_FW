#include "bsp.h"

#include "led/led.h"
#include "power/power.h"

bool bsp_init(void) {
  power_init();
  led_init();

  return true;
}