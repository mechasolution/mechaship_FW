#include "mw.h"

#include "tusb/tusb_.h"
#include "uros/uros.h"

bool mw_init(void) {
  bool ret = true;

  ret &= mw_tusb_init();
  ret &= uros_init();

  return ret;
}