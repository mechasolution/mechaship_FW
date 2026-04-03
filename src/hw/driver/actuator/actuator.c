/**
 * @file actuator.c
 * @note Actuator에는 Thruster, Key 뿐만 아니라 LED 등도 포함되지만, 모듈화의 편의를 위해
 *       Thruster, Key를 묶어 Actuator로 취급함
 */

#include <zephyr/drivers/pwm.h>

#include "hwconf.h"
#include "actuator.h"

static const struct pwm_dt_spec s_servo = HWCONF_SERVO_PWM_SPEC;
static const struct pwm_dt_spec s_esc = HWCONF_ESC_PWM_SPEC;

static uint16_t s_key_pulse_0_degree = 500;
static uint16_t s_key_pulse_180_degree = 2500;
static uint16_t s_key_pulse_now;
static float s_key_min;
static float s_key_max = 180;

static uint16_t s_thruster_pulse_0_percentage = 1500;
static uint16_t s_thruster_pulse_100_percentage = 1900;
static uint16_t s_thruster_pulse_now;

static inline int s_set_pulse_us(const struct pwm_dt_spec *spec, uint16_t pulse_us) {
  return pwm_set(spec->dev, spec->channel, spec->period, PWM_USEC(pulse_us), spec->flags);
}

bool actuator_init(void) {
  if (!pwm_is_ready_dt(&s_servo) || !pwm_is_ready_dt(&s_esc)) {
    return false;
  }

  s_key_min = 0;
  s_key_pulse_now = s_key_pulse_0_degree;
  s_thruster_pulse_now = s_thruster_pulse_0_percentage;

  return s_set_pulse_us(&s_servo, s_key_pulse_now) == 0 && s_set_pulse_us(&s_esc, s_thruster_pulse_now) == 0;
}

void actuator_pwm_off(void) {
  (void)s_set_pulse_us(&s_servo, 0);
  (void)s_set_pulse_us(&s_esc, 0);
}

void actuator_set_key_info(uint16_t pulse_0, uint16_t pulse_180, float min_degree, float max_degree) {
  s_key_pulse_0_degree = pulse_0;
  s_key_pulse_180_degree = pulse_180;
  s_key_min = min_degree;
  s_key_max = max_degree;
}

void actuator_set_thruster_info(uint16_t pulse_0, uint16_t pulse_100) {
  s_thruster_pulse_0_percentage = pulse_0;
  s_thruster_pulse_100_percentage = pulse_100;
}

float actuator_get_key_degree(void) {
  if (s_key_pulse_180_degree == s_key_pulse_0_degree) {
    return 0.0f;
  }

  float degree = ((float)(s_key_pulse_now - s_key_pulse_0_degree) /
                  (float)(s_key_pulse_180_degree - s_key_pulse_0_degree)) *
                 180.0f;

  if (degree < 0.0f) {
    degree = 0.0f;
  } else if (degree > 180.0f) {
    degree = 180.0f;
  }

  return degree;
}

void actuator_set_key_degree(float degree) {
  if (degree < s_key_min) {
    degree = s_key_min;
  } else if (degree > s_key_max) {
    degree = s_key_max;
  }

  uint16_t pulse = s_key_pulse_0_degree +
                   (uint16_t)((s_key_pulse_180_degree - s_key_pulse_0_degree) * (degree / 180.0f));

  (void)s_set_pulse_us(&s_servo, pulse);
  s_key_pulse_now = pulse;
}

void actuator_set_key_pulse(uint16_t us) {
  (void)s_set_pulse_us(&s_servo, us);
  s_key_pulse_now = us;
}

uint16_t actuator_get_key_pulse(void) {
  return s_key_pulse_now;
}

float actuator_get_thruster_percentage(void) {
  if (s_thruster_pulse_100_percentage == s_thruster_pulse_0_percentage) {
    return 0.0f;
  }

  float percentage = ((float)(s_thruster_pulse_now - s_thruster_pulse_0_percentage) /
                      (float)(s_thruster_pulse_100_percentage - s_thruster_pulse_0_percentage)) *
                     100.0f;

  if (percentage < -100.0f) {
    percentage = -100.0f;
  } else if (percentage > 100.0f) {
    percentage = 100.0f;
  }

  return percentage;
}

void actuator_set_thruster_percentage(float percentage) {
  if (percentage < -100.0f) {
    percentage = -100.0f;
  } else if (percentage > 100.0f) {
    percentage = 100.0f;
  }

  uint16_t pulse = s_thruster_pulse_0_percentage +
                   (int16_t)((s_thruster_pulse_100_percentage - s_thruster_pulse_0_percentage) *
                             (percentage / 100.0f));

  (void)s_set_pulse_us(&s_esc, pulse);
  s_thruster_pulse_now = pulse;
}

void actuator_set_thruster_pulse(uint16_t us) {
  (void)s_set_pulse_us(&s_esc, us);
  s_thruster_pulse_now = us;
}

uint16_t actuator_get_thruster_pulse(void) {
  return s_thruster_pulse_now;
}
