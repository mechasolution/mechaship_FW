#include "app/app.h"
#include "hw/hw.h"
#include "hw/driver/power/power.h"
#include "mw/mw.h"

int main(void) {
  if (!hw_init()) return 1;

  app_start_sequence();

  if (!mw_init() || !app_init()) {
    power_set_sbc(false);
    power_set_main(false);
  }

  app_main();

  return 0;
}
