#include "driver.h"

#include "actuator/actuator.h"
#include "battery/battery.h"
#include "emo/emo.h"
#include "lcd/lcd.h"
#include "led/led.h"
#include "log/log.h"
#include "power/power.h"
#include "rc4/rc4.h"
#include "rgbw_led/rgbw_led.h"
#include "switch8/switch8.h"
#include "tone/tone.h"

bool driver_init(void) {
  bool ret = true;

  ret &= actuator_init();
  ret &= battery_init();
  ret &= emo_init();
  ret &= led_init();
  ret &= lcd_init();
  ret &= log_init();
  ret &= power_init();
#ifdef __ZEPHYR__
  /* RC4 / RGBW are still legacy PIO implementations. Keep init best-effort. */
  (void)rc4_init();
  (void)rgbw_led_init();
#else
  ret &= rc4_init();
  ret &= rgbw_led_init();
#endif
  ret &= switch8_init();
  ret &= tone_init();

  return ret;
}
