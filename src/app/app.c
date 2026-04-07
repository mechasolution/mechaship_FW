#include "temp_task/temp_task.h"

#include "app.h"

bool app_init(void) {
  bool ret = true;

  rtos_init();

  return ret;
}

void app_start(void) {
  temp_task_start();
}
