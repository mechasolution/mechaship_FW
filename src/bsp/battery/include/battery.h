#ifndef __BSP_BATTERY_H__
#define __BSP_BATTERY_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t pin;
  uint8_t adc_resolution; // ADC 분해능

  double adc_voltage_max; // ADC 최대 입력 전압

  double max_voltage; // 0~(ADC 최대 입력 전압) 기준 100%
  double min_voltage; // 0~(ADC 최대 입력 전압) 기준 0%
} bsp_battery_conf_t;

bool bsp_battery_init(bsp_battery_conf_t *conf_p_);
bool bsp_battery_get(double *voltage_p_, double *percentage_p_);

#endif