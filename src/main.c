#include "app/app.h"
#include "hw/hw.h"
#include "mw/mw.h"

int main(void) {
  hw_init();

  app_start_sequence();

  mw_init();
  app_init();

  app_main();

  return 0;
}
