#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "driver/led/led.h"
#include "driver/log/log.h"

#include "sled_task.h"

#define TAG "sled"

typedef sled_task_pattern_t sled_task_queue_data_t;
#define SLED_TASK_QUEUE_LENGTH 2
#define SLED_TASK_QUEUE_ITEM_SIZE sizeof(sled_task_queue_data_t)
static QueueHandle_t s_sled_task_queue_hd = NULL;
static uint8_t s_sled_task_queue_buff[SLED_TASK_QUEUE_LENGTH * SLED_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_sled_task_queue_struct;

#define SLED_TASK_SIZE configMINIMAL_STACK_SIZE
static StackType_t s_sled_task_buff[SLED_TASK_SIZE];
static StaticTask_t s_sled_task_struct;

static void s_sled_task(void *arg) {
  sled_task_queue_data_t queue_data;
  uint8_t pattern_idx = 0;
  bool pattern[SLED_TASK_PATTERN_MAX][12] = {
      {false, false, false, false, false, false, false, false, false, false, false, false}, // SLED_TASK_PATTERN_OFF
      {true, true, true, true, true, true, true, true, true, true, true, true},             // SLED_TASK_PATTERN_SOLID

      {true, true, true, true, true, true, false, false, false, false, false, false}, // SLED_TASK_PATTERN_BLINK_SLOW
      {true, false, true, false, true, false, true, false, true, false, true, false}, // SLED_TASK_PATTERN_BLINK_FAST

      {true, false, false, false, false, false, false, false, false, false, false, false}, // SLED_TASK_PATTERN_1
      {false, true, true, true, true, true, true, true, true, true, true, true},           // SLED_TASK_PATTERN_2

  };

  queue_data = SLED_TASK_PATTERN_OFF;

  for (;;) {
    if (xQueueReceive(s_sled_task_queue_hd, &queue_data, 100 / portTICK_PERIOD_MS) == pdTRUE) {
      if (queue_data >= SLED_TASK_PATTERN_MAX || queue_data <= SLED_TASK_PATTERN_OFF) {
        queue_data = SLED_TASK_PATTERN_OFF;
      }
    }

    led_set_s(pattern[queue_data][pattern_idx]);

    pattern_idx = pattern_idx >= 11 ? 0 : pattern_idx + 1;
  }
}

bool sled_task_init(void) {
  s_sled_task_queue_hd = xQueueCreateStatic(
      SLED_TASK_QUEUE_LENGTH,
      SLED_TASK_QUEUE_ITEM_SIZE,
      s_sled_task_queue_buff,
      &s_sled_task_queue_struct);

  xTaskCreateStatic(
      s_sled_task,
      "sled",
      SLED_TASK_SIZE,
      NULL,
      configIDLE_TASK_PRIORITIES,
      s_sled_task_buff,
      &s_sled_task_struct);

  return true;
}

bool sled_task_set_pattern(sled_task_pattern_t pattern) {
  bool ret = xQueueSend(s_sled_task_queue_hd, &pattern, 0) == pdPASS;
  if (ret == false) {
    log_warning(TAG, "Publish queue full!! message dropped!!");
  }

  return ret;
}