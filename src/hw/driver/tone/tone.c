#include <zephyr/drivers/pwm.h>

#include "hwconf.h"
#include "tone.h"

static const struct pwm_dt_spec s_piezo = HWCONF_PIEZO_PWM_SPEC;

bool tone_init(void) {
  if (!pwm_is_ready_dt(&s_piezo)) {
    return false;
  }

  return pwm_set_dt(&s_piezo, 0) == 0;
}

void tone_set(uint16_t hz) {
  if (hz == 0U) {
    (void)pwm_set_dt(&s_piezo, 0);
    return;
  }

  uint32_t period_ns = (uint32_t)(1000000000ULL / hz);
  uint32_t pulse_ns = period_ns / 2U;

  (void)pwm_set(s_piezo.dev, s_piezo.channel, period_ns, pulse_ns, s_piezo.flags);
}
