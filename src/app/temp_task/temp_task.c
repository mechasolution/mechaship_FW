#include <zephyr/kernel.h>

#include "temp_task.h"

#include "led/led.h"
#include "power/power.h"

static void s_task(void *, void *, void *) {
  power_set_main(true);

  led_test(true);

  k_msleep(2000);

  led_test(false);

  for (;;) {
    k_msleep(1000);
  }
}

K_THREAD_DEFINE(temp_task, 500,
                s_task, NULL, NULL, NULL,
                5, 0, SYS_FOREVER_MS);

void temp_task_start(void) {
  k_thread_start(temp_task);
}

// void temp_task_stop(void) {
//   ;
// }
