#include <FreeRTOS.h>
#include <task.h>

#include <tusb.h>

#include "tusb_.h"

#define TUSB_TASK_SIZE (4096)
static TaskHandle_t s_tusb_task_hd = NULL;
static StackType_t s_tusb_task_buff[TUSB_TASK_SIZE];
static StaticTask_t s_tusb_task_struct;

static void s_tusb_task(void *arg) {
  (void)arg;

  tusb_init();

  for (;;) {
    tud_task();
  }
}

bool mw_tusb_init(void) {
  s_tusb_task_hd = xTaskCreateStatic(
      s_tusb_task,
      "tusb",
      TUSB_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES - 1,
      s_tusb_task_buff,
      &s_tusb_task_struct);

  return s_tusb_task_hd != NULL;
}