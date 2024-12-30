#include "hal.h"

#include "time/time.h"

bool hal_init(void) {
  bool ret = true;

  ret &= time_init();

  return ret;
}
