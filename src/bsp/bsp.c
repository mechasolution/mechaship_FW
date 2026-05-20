#include "bsp.h"

#include "led/led.h"
#include "power/power.h"
#include "tone/tone.h"
#include "actuator/actuator.h"

bool bsp_init(void) {
  power_init();
  led_init();
  tone_init();
  actuator_init();

  return true;
}