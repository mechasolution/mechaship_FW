#include <zephyr/kernel.h>

#include "temp_task.h"

#include "actuator/actuator.h"
#include "led/led.h"
#include "power/power.h"
#include "tone/tone.h"

static void s_test_start(void) {
  led_test(true);
}

static void s_test_stop(void) {
  led_test(false);
}

static void s_task(void *, void *, void *) {
  // beeep
  tone_set(741);
  k_msleep(100);
  tone_reset();

  // 테스트 상태 on
  s_test_start();

  // 테스트 상태 2초 유지
  k_msleep(2000);

  // 버튼 손 땔때까지 테스트 상태 유지
  while (power_get_button()) {
    k_msleep(50);
  }

  // 테스트 상태 종료
  s_test_stop();

  // 상태 LED blink
  k_msleep(500);
  for (int i = 0; i < 3; i++) {
    led_set_s(true);
    k_msleep(80);
    led_set_s(false);
    k_msleep(80);
  }

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
