#include "hw.h"

#include "driver/driver.h"
#include "hal/hal.h"

bool hw_init(void) {
  bool ret = true;

  ret &= hal_init();
  ret &= driver_init();

  return ret;
}
