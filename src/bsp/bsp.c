#include "bsp.h"

#include "actuator/actuator.h"
#include "led/led.h"
#include "power/power.h"
#include "rc4/rc4.h"
#include "tone/tone.h"

bool bsp_init(void) {
  power_init();
  led_init();
  tone_init();
  actuator_init();
  rc4_init();

  return true;
}