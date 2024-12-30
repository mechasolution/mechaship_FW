#include "hardware/pwm.h"
#include "pico/stdlib.h"

#include "hwconf.h"

#include "tone.h"

#define SYSCLOCK 125000000

static pwm_config s_cfg;
static uint s_slice_num;

bool tone_init(void) {
  gpio_set_function(HWCONF_PIN_PIEZO, GPIO_FUNC_PWM);
  s_slice_num = pwm_gpio_to_slice_num(HWCONF_PIN_PIEZO);

  s_cfg = pwm_get_default_config();

  return true;
}

void tone_set(uint16_t hz) {
  if (hz == 0) {
    pwm_set_chan_level(s_slice_num, PWM_CHAN_B, 0);
    return;
  }

  uint count = (SYSCLOCK * 16) / hz;
  uint div;

  // 미리 계산된 상수 16 * 60000 = 960000 사용
  if (count < 960000) {
    div = 16;
  } else {
    div = count / 60000;
  }

  s_cfg.div = div;
  s_cfg.top = count / div;

  pwm_set_enabled(s_slice_num, false);
  pwm_init(s_slice_num, &s_cfg, true);
  pwm_set_chan_level(s_slice_num, PWM_CHAN_B, s_cfg.top / 2);
  pwm_set_enabled(s_slice_num, true);
}
