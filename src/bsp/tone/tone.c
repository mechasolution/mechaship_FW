#include <zephyr/drivers/pwm.h>

#include "tone.h"

static const struct pwm_dt_spec s_piezo = PWM_DT_SPEC_GET(DT_NODELABEL(buzzer_pwm));

bool tone_init(void) {
  if (!pwm_is_ready_dt(&s_piezo)) {
    return false;
  }

  return pwm_set_dt(&s_piezo, PWM_HZ(1000), 0) == 0;
}

void tone_set(uint16_t hz) {
  if (hz == 0U) {
    (void)pwm_set_dt(&s_piezo, PWM_HZ(1000), 0);
    return;
  }

  pwm_set_dt(&s_piezo, PWM_HZ(hz), PWM_HZ(hz) / 2);
}
