/**
 * @file actuator.c
 * @note Actuator에는 Thruster, Key 뿐만 아니라 LED 등도 포함되지만, 모듈화의 편의를 위해
 *       Thruster, Key를 묶어 Actuator로 취급함
 *
 */

#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>

#include "actuator.h"

#define PWM_FREQ_HZ 50
#define PWM_WRAP 20000
#define SLICE_SERVO pwm_gpio_to_slice_num(HWCONF_PIN_PWM_SERVO)
#define SLICE_ESC pwm_gpio_to_slice_num(HWCONF_PIN_PWM_ESC)

static uint16_t s_key_pulse_0_degree = 500;    // 0도일 때 펄스
static uint16_t s_key_pulse_180_degree = 2500; // 180도일 때 펄스
static uint16_t s_key_pulse_now = 0;
static float s_key_min = 0;
static float s_key_max = 180;

static uint16_t s_thruster_pulse_0_percentage = 1500;   // Thruster 중지할 때 펄스
static uint16_t s_thruster_pulse_100_percentage = 1900; // 최대 속도일 때 펄스
static uint16_t s_thruster_pulse_now = 0;

bool actuator_init(void) {
  gpio_set_function(HWCONF_PIN_PWM_SERVO, GPIO_FUNC_PWM);
  gpio_set_function(HWCONF_PIN_PWM_ESC, GPIO_FUNC_PWM);

  pwm_config cfg = pwm_get_default_config();

  uint32_t div = clock_get_hz(clk_sys) / (PWM_WRAP * PWM_FREQ_HZ);

  pwm_config_set_clkdiv(&cfg, (float)div);
  pwm_config_set_wrap(&cfg, PWM_WRAP);

  pwm_init(SLICE_SERVO, &cfg, true);
  pwm_init(SLICE_ESC, &cfg, true);

  return true;
}

void actuator_pwm_off(void) {
  pwm_set_gpio_level(HWCONF_PIN_PWM_SERVO, 0);
  pwm_set_gpio_level(HWCONF_PIN_PWM_ESC, 0);
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

  uint16_t pulse = s_key_pulse_0_degree + (uint16_t)((s_key_pulse_180_degree - s_key_pulse_0_degree) * (degree / 180.0f));

  pwm_set_gpio_level(HWCONF_PIN_PWM_SERVO, pulse);

  s_key_pulse_now = pulse;
}

void actuator_set_key_pulse(uint16_t us) {
  pwm_set_gpio_level(HWCONF_PIN_PWM_SERVO, us);
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

  uint16_t pulse = s_thruster_pulse_0_percentage + (int16_t)((s_thruster_pulse_100_percentage - s_thruster_pulse_0_percentage) * (percentage / 100.0f));

  pwm_set_gpio_level(HWCONF_PIN_PWM_ESC, pulse);

  s_thruster_pulse_now = pulse;
}

void actuator_set_thruster_pulse(uint16_t us) {
  pwm_set_gpio_level(HWCONF_PIN_PWM_ESC, us);
  s_thruster_pulse_now = us;
}

uint16_t actuator_get_thruster_pulse(void) {
  return s_thruster_pulse_now;
}
