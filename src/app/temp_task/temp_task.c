#include <zephyr/kernel.h>

#include "temp_task.h"

#include "led/led.h"

void temp_task_start(void) {
  led_test(true);

  k_msleep(2000);

  led_test(false);
}

void temp_task_stop(void) {
  ;
}
