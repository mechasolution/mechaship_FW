#include "include/key.h"

static long s_map(long x, long in_min, long in_max, long out_min, long out_max) {
  const long run = in_max - in_min;
  if (run == 0) {
    log_e("map(): Invalid input range, min == max");
    return -1; // AVR returns -1, SAM returns 0
  }
  const long rise = out_max - out_min;
  const long delta = x - in_min;
  return (delta * rise) / run + out_min;
}

bool bsp_key_init(bsp_key_handle_t *handler_p_) {
  if (handler_p_->is_init == true) {
    return false;
  }
  handler_p_->is_init = true;

  Servo *servo_p = new Servo();
  handler_p_->servo_h = servo_p;

  servo_p->attach(handler_p_->pin_num);

  return true;
}

bool bsp_key_set(bsp_key_handle_t *handler_p_, uint16_t degree_) {
  if (handler_p_->is_init == false) {
    return false;
  }

  handler_p_->servo_h->write(degree_);

  return true;
}
