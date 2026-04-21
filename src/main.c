#include "app/app.h"
#include "bsp/bsp.h"

int main(void) {
  bsp_init();

  app_init();

  app_start();

  return 0;
}
