#include "app.h"
#include "rtos/rtos.h"

#include "driver/battery/battery.h"
#include "driver/lcd/lcd.h"
#include "driver/led/led.h"
#include "driver/power/power.h"
#include "driver/rgbw_led/rgbw_led.h"
#include "driver/tone/tone.h"

#include "hal/time/time.h"

bool app_init(void) {
  bool ret = true;

  rtos_init();

  return ret;
}

static void check_voltage(void) {
  if (battery_get_percentage() <= 0) {
    lcd_set_string("Power off in -");
    lcd_set_cursor(1, 0);
    lcd_set_string("LOW POWER!!!");

    tone_set(2000);
    time_block_ms(3000);

    power_set_main(false);
  }
}

static void start_sequence(void) {
  lcd_set_string("0123456789abcdef");
  lcd_set_cursor(1, 0);
  lcd_set_string("ghijklmnopqrstuv");
  lcd_next_frame();

  led_set_1(true), led_set_2(true), led_set_f(true), led_set_s(true);
  rgbw_led_set_pixels(rgbw_led_get_color(5, 5, 5, 5));
  tone_set(741);

  time_block_ms(100);

  tone_reset();

  time_block_ms(1900);

  while (power_get_button()) {
    ; // block until switch released
  }

  lcd_clear();
  lcd_next_frame();

  led_set_1(false), led_set_2(false), led_set_f(false), led_set_s(false);
  rgbw_led_clear_all();

  time_block_ms(500);

  for (int i = 0; i < 3; i++) {
    led_set_1(true);
    time_block_ms(80);
    led_set_1(false);
    time_block_ms(80);
  }
}

#include "driver/actuator/actuator.h"

void app_main(void) {
  power_set_main(true);

  check_voltage();

  power_set_sbc(true);

  start_sequence();

  rtos_start();
}
