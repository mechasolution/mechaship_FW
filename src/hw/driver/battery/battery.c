#include <math.h>

#include <hardware/adc.h>
#include <hardware/gpio.h>

#include "battery.h"

#define BATTERY_MAX_VOLTAGE 4.2 * HWCONF_BATTERY_CELL_CNT
#define BATTERY_MIN_VOLTAGE HWCONF_BATTERY_TOTAL_LOW_ALERT_VOLTAGE

bool battery_init(void) {
  adc_init();
  adc_gpio_init(HWCONF_PIN_VOLTAGE_BATTERY);
  adc_select_input(3);
  return true;
}

float battery_get_voltage(void) {
  return adc_read() * (3.3 / 4095.0) * HWCONF_BATTERY_VOLTAGE_DEVIDER;
}

float battery_get_percentage(void) {
  double voltage = battery_get_voltage();

  if (voltage >= BATTERY_MAX_VOLTAGE) {
    return 100.0f;
  } else if (voltage <= BATTERY_MIN_VOLTAGE) {
    return 0.0f;
  } else {
    return ((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100.0f;
  }
}
