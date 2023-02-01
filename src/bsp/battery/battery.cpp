#include <math.h>
#include <string.h>

#include <Arduino.h>

#include "include/battery.h"

static bsp_battery_conf_t s_conf;

static bool s_init;

static double s_map_d(double x, double in_min, double in_max, double out_min, double out_max) {
  const double run = in_max - in_min;
  if (run == 0) {
    return -1; // AVR returns -1, SAM returns 0
  }
  const double rise = out_max - out_min;
  const double delta = x - in_min;
  return (delta * rise) / run + out_min;
}

bool bsp_battery_init(bsp_battery_conf_t *conf_p_) {
  if (conf_p_ == NULL) {
    return false;
  }
  if (s_init == true) {
    return false;
  }
  s_init = true;

  memcpy(&s_conf, conf_p_, sizeof(bsp_battery_conf_t));

  analogReadResolution(s_conf.adc_resolution);
  pinMode(s_conf.pin_20p_led, OUTPUT);
  pinMode(s_conf.pin_40p_led, OUTPUT);
  pinMode(s_conf.pin_60p_led, OUTPUT);
  pinMode(s_conf.pin_80p_led, OUTPUT);

  return true;
}

bool bsp_battery_get(double *voltage_p_, double *percentage_p_) {
  if (s_init == false) {
    return false;
  }
  if (voltage_p_ == NULL || percentage_p_ == NULL) {
    return false;
  }

  uint16_t ret = analogRead(s_conf.pin_adc);
  double voltage = s_map_d(ret, 0, pow(2, s_conf.adc_resolution), 0, s_conf.adc_voltage_max);
  *voltage_p_ = voltage;

  double percentage = s_map_d(voltage, s_conf.min_voltage, s_conf.max_voltage, 0, 100);
  *percentage_p_ = constrain(percentage, 0, 100);

  return true;
}

void bsp_battery_update_led(void) {
  double voltage, percentage;
  bsp_battery_get(&voltage, &percentage);

  if (percentage >= 80) {
    digitalWrite(s_conf.pin_80p_led, HIGH);
    digitalWrite(s_conf.pin_60p_led, HIGH);
    digitalWrite(s_conf.pin_40p_led, HIGH);
    digitalWrite(s_conf.pin_20p_led, HIGH);
  } else if (percentage >= 60) {
    digitalWrite(s_conf.pin_80p_led, LOW);
    digitalWrite(s_conf.pin_60p_led, HIGH);
    digitalWrite(s_conf.pin_40p_led, HIGH);
    digitalWrite(s_conf.pin_20p_led, HIGH);
  } else if (percentage >= 40) {
    digitalWrite(s_conf.pin_80p_led, LOW);
    digitalWrite(s_conf.pin_60p_led, LOW);
    digitalWrite(s_conf.pin_40p_led, HIGH);
    digitalWrite(s_conf.pin_20p_led, HIGH);
  } else if (percentage >= 20) {
    digitalWrite(s_conf.pin_80p_led, LOW);
    digitalWrite(s_conf.pin_60p_led, LOW);
    digitalWrite(s_conf.pin_40p_led, LOW);
    digitalWrite(s_conf.pin_20p_led, HIGH);
  } else {
    digitalWrite(s_conf.pin_80p_led, LOW);
    digitalWrite(s_conf.pin_60p_led, LOW);
    digitalWrite(s_conf.pin_40p_led, LOW);
    digitalWrite(s_conf.pin_20p_led, LOW);
  }
}
