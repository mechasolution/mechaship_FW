#include "mw.h"

#include "tusb/tusb_.h"
#include "uros/uros.h"

bool mw_init(void) {

  mw_tusb_init();
  uros_init();

  return true;
}