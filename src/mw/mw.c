#include "mw.h"

#include "sbc/sbc.h"
#include "tusb/tusb_.h"

bool mw_init(void) {
  bool ret = true;

  ret &= mw_tusb_init();
  ret &= mw_sbc_init();
  // to not init uros here; init when ROS mode turned on

  return ret;
}