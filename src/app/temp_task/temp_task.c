#include <zephyr/kernel.h>

#include "temp_task.h"

#include "led/led.h"
#include "power/power.h"
#include "tone/tone.h"

static void s_task(void *, void *, void *) {
  power_set_main(true);

  tone_set(741);
  k_msleep(100);
  tone_reset();

  led_test(true);

  k_msleep(2000);

  while (power_get_button()) {
    k_msleep(50);
  }

  led_test(false);

  k_msleep(500);
  for (int i = 0; i < 3; i++) {
    led_set_s(true);
    k_msleep(80);
    led_set_s(false);
    k_msleep(80);
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
