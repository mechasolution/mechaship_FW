#include <FreeRTOS.h>
#include <task.h> // FreeRTOS

#include "central_task/central_task.h"
#include "peripheral_task/actuator_task.h"
#include "peripheral_task/lcd_task.h"
#include "peripheral_task/sled_task.h"

#include "rtos.h"

#include "driver/log/log.h"

#define TAG "RTOS"

static void s_task_init(void) {
  bool result;

  result = central_task_init();
  configASSERT(result);

  result = actuator_task_init();
  configASSERT(result);

  result = sled_task_init();
  configASSERT(result);

  result = lcd_task_init();
  configASSERT(result);
}

void rtos_init(void) {
  s_task_init();
}

void rtos_start(void) {
  vTaskStartScheduler();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)pcTaskName;
  (void)xTask;

  log_error(TAG, "Stack overflow while running task: %s", pcTaskName);

  /* Run time stack overflow checking is performed if
  configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
  function is called if a stack overflow is detected.  pxCurrentTCB can be
  inspected in the debugger if the task name passed into this function is
  corrupt. */
  for (;;) {
    __breakpoint();
  }
}

void __assert_func(const char *file, int line, const char *func, const char *failedexpr) {
  log_error(TAG, "assertion \"%s\" failed: file \"%s\", line %d%s%s\n",
            failedexpr, file, line, func ? ", function: " : "",
            func ? func : "");

  for (;;) {
    __breakpoint();
  }
}
