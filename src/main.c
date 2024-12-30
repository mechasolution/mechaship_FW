#include "app/app.h"
#include "hw/hw.h"
#include "mw/mw.h"

#include "driver/log/log.h"

int main(void) {
  mw_init();

  hw_init();
  app_init();

  app_main();

  return 0;
}
