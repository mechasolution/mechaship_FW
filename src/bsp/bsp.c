#include "bsp.h"

#include "led/led.h"
#include "power/power.h"
#include "tone/tone.h"

bool bsp_init(void) {
  power_init();
  led_init();
  tone_init();

  return true;
}