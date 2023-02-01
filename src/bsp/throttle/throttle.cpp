#include <stddef.h>

#include "include/throttle.h"

static long s_map(long x, long in_min, long in_max, long out_min, long out_max) {
  const long run = in_max - in_min;
  if (run == 0) {
    return -1; // AVR returns -1, SAM returns 0
  }
  const long rise = out_max - out_min;
  const long delta = x - in_min;
  return (delta * rise) / run + out_min;
}

bool bsp_throttle_init(bsp_throttle_handle_t *handler_p_) {
  if (handler_p_->is_init == true) {
    return false;
  }
  handler_p_->is_init = true;

  handler_p_->servo_h = new Servo();

  handler_p_->servo_h->attach(handler_p_->pin_num);

  return true;
}

bool bsp_throttle_set_by_percentage(bsp_throttle_handle_t *handler_p_, uint8_t percentage_) {
  if (handler_p_->is_init == false || handler_p_->is_emo == true) {
    return false;
  }

  if (percentage_ > 100) {
    percentage_ = 100;
  }
  int deg = s_map(percentage_, 0, 100, handler_p_->min_pwr_pulse_width, handler_p_->max_pwr_pulse_width);
  handler_p_->servo_h->writeMicroseconds(deg);

  return true;
}

bool bsp_throttle_set_by_pulse_width(bsp_throttle_handle_t *handler_p_, uint16_t pulse_width_) {
  if (handler_p_->is_init == false || handler_p_->is_emo == true) {
    return false;
  }

  handler_p_->servo_h->writeMicroseconds(pulse_width_);

  return true;
}

void bsp_throttle_set_emo(bsp_throttle_handle_t *handler_p_) {
  if (handler_p_ == NULL) {
    return;
  }
  bsp_throttle_set_by_percentage(handler_p_, 0);
  handler_p_->is_emo = true;
}

void bsp_throttle_reset_emo(bsp_throttle_handle_t *handler_p_) {
  if (handler_p_ == NULL) {
    return;
  }
  handler_p_->is_emo = false;
}
