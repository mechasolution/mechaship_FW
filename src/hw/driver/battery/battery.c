#include <math.h>

#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>

#include "hwconf.h"
#include "battery.h"

static struct adc_dt_spec s_adc = HWCONF_BATTERY_ADC_SPEC;
static struct k_mutex s_adc_mutex;

#define BATTERY_MAX_VOLTAGE (4.2f * HWCONF_BATTERY_CELL_CNT)
#define BATTERY_MIN_VOLTAGE (HWCONF_BATTERY_TOTAL_LOW_ALERT_VOLTAGE)

bool battery_init(void) {
  if (!adc_is_ready_dt(&s_adc)) {
    return false;
  }

  if (adc_channel_setup_dt(&s_adc) < 0) {
    return false;
  }

  k_mutex_init(&s_adc_mutex);

  return true;
}

float battery_get_voltage(void) {
  int16_t raw = 0;
  struct adc_sequence seq = {
      .buffer = &raw,
      .buffer_size = sizeof(raw),
  };

  (void)adc_sequence_init_dt(&s_adc, &seq);

  k_mutex_lock(&s_adc_mutex, K_FOREVER);
  int err = adc_read_dt(&s_adc, &seq);
  k_mutex_unlock(&s_adc_mutex);

  if (err < 0) {
    return 0.0f;
  }

  int32_t mv = raw;
  err = adc_raw_to_millivolts_dt(&s_adc, &mv);
  if (err < 0) {
    return 0.0f;
  }

  return ((float)mv / 1000.0f) * HWCONF_BATTERY_VOLTAGE_DEVIDER;
}

float battery_get_percentage(void) {
  double voltage = battery_get_voltage();

  if (voltage >= BATTERY_MAX_VOLTAGE) {
    return 100.0f;
  }
  if (voltage <= BATTERY_MIN_VOLTAGE) {
    return 0.0f;
  }

  return (float)(((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100.0f);
}
