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

static bool s_test_battery(void) {
  return battery_get_percentage() > 5;
}

static bool s_test_hw(void) {
  led_test(true);
  rgbw_led_test(true);
  lcd_test(true);

  tone_set(741);
  time_block_ms(100);
  tone_reset();

  time_block_ms(1900);

  while (power_get_button()) {
    ; // block until switch released
  }

  led_test(false);
  rgbw_led_test(false);
  lcd_test(false);

  time_block_ms(500);

  for (int i = 0; i < 3; i++) {
    led_set_s(true);
    time_block_ms(80);
    led_set_s(false);
    time_block_ms(80);
  }

  return true;
}

void app_start_sequence(void) {
  power_set_main(true);

  if (s_test_battery() == false) {
    lcd_set_string("Power off in -");
    lcd_set_cursor(1, 0);
    lcd_set_string("LOW POWER!!!");
    lcd_next_frame();

    tone_set(2000);
    time_block_ms(3000);

    power_set_main(false); // stop MCU
  }

  if (s_test_hw() == false) {
    lcd_set_string("Power off in -");
    lcd_set_cursor(1, 0);
    lcd_set_string("HW init failed!!!");
    lcd_next_frame();

    tone_set(2000);
    time_block_ms(3000);

    power_set_main(false); // stop MCU
  }

  power_set_sbc(true);
}

void app_main(void) {
  rtos_start();
}
