#include "temp_task/temp_task.h"

#include "app.h"

bool app_init(void) {
  return true;
}

void app_start(void) {
  temp_task_start();
}
