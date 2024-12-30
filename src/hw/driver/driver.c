#include "driver.h"

#include "actuator/actuator.h"
#include "battery/battery.h"
#include "emo/emo.h"
#include "lcd/lcd.h"
#include "led/led.h"
#include "log/log.h"
#include "power/power.h"
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
  ret &= rgbw_led_init();
  ret &= switch8_init();
  ret &= tone_init();

  return ret;
}
