#include <pico/stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdatomic.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

#include <rmw_microros/rmw_microros.h>

#include <rcl/error_handling.h>
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>

#include "driver/log/log.h"

#include "support.h"
#include "uros.h"

#define TAG "mw/microros"

typedef struct {
  uros_pub_data_flag_t data_flag;
  uros_pub_data_t data;
} mw_uros_task_queue_data_t;

#define MW_UROS_TASK_QUEUE_LENGTH 10
#define MW_UROS_TASK_QUEUE_ITEM_SIZE sizeof(mw_uros_task_queue_data_t)
static QueueHandle_t mw_uros_task_queue_hd = NULL;
static uint8_t s_mw_uros_task_queue_buff[MW_UROS_TASK_QUEUE_LENGTH * MW_UROS_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_mw_uros_task_queue_struct;

#define MW_UROS_TASK_SIZE 4096
static TaskHandle_t s_mw_uros_task_hd = NULL;
static StackType_t s_mw_uros_task_buff[MW_UROS_TASK_SIZE];
static StaticTask_t s_mw_uros_task_struct;
static SemaphoreHandle_t s_stop_ack;
static StaticSemaphore_t s_stop_ack_storage;
static atomic_bool s_running = false;
static atomic_bool s_stopped = true;
static bool s_stop_result;

#define UROS_NOTIFY_START (1U << 0)
#define UROS_NOTIFY_STOP  (1U << 1)

static void s_uros_task(void *arg) {
  (void)arg;

  mw_uros_task_queue_data_t queue_buff;

  agent_init();

  for (;;) {
    uint32_t signals = 0;
    xTaskNotifyWait(0, UINT32_MAX, &signals, portMAX_DELAY);
    if ((signals & UROS_NOTIFY_START) == 0) continue;

    bool stop = (signals & UROS_NOTIFY_STOP) != 0;
    while (!stop) {
      signals = 0;
      xTaskNotifyWait(0, UINT32_MAX, &signals, 0);
      if (signals & UROS_NOTIFY_STOP) break;

      agent_spin();
      while (agent_is_connected() && xQueueReceive(mw_uros_task_queue_hd, &queue_buff, 0) == pdTRUE) {
        signals = 0;
        xTaskNotifyWait(0, UINT32_MAX, &signals, 0);
        if (signals & UROS_NOTIFY_STOP) {
          stop = true;
          break;
        }
        if (!publisher_publish(queue_buff.data_flag, &queue_buff.data)) {
          entity_destroy();
          agent_reset();
          break;
        }
      }
    }
    s_stop_result = entity_destroy();
    agent_reset();
    xQueueReset(mw_uros_task_queue_hd);
    atomic_store(&s_stopped, true);
    xSemaphoreGive(s_stop_ack);
  }
}

bool uros_init(void) {
  if (!atomic_load(&s_stopped) || atomic_load(&s_running)) return false;

  if (mw_uros_task_queue_hd == NULL) {
    mw_uros_task_queue_hd = xQueueCreateStatic(
        MW_UROS_TASK_QUEUE_LENGTH,
        MW_UROS_TASK_QUEUE_ITEM_SIZE,
        s_mw_uros_task_queue_buff,
        &s_mw_uros_task_queue_struct);
  }
  if (mw_uros_task_queue_hd == NULL) return false;

  if (s_stop_ack == NULL) s_stop_ack = xSemaphoreCreateBinaryStatic(&s_stop_ack_storage);
  if (s_stop_ack == NULL) return false;

  if (s_mw_uros_task_hd == NULL) {
    s_mw_uros_task_hd = xTaskCreateStaticAffinitySet(
        s_uros_task,
        "uros",
        MW_UROS_TASK_SIZE,
        NULL,
        configMAX_PRIORITIES - 2,
        s_mw_uros_task_buff,
        &s_mw_uros_task_struct,
        1U << 0);
  }
  if (s_mw_uros_task_hd == NULL) return false;

  atomic_store(&s_stopped, false);
  atomic_store(&s_running, true);
  if (xTaskNotify(s_mw_uros_task_hd, UROS_NOTIFY_START, eSetBits) != pdPASS) {
    atomic_store(&s_running, false);
    atomic_store(&s_stopped, true);
    return false;
  }
  return true;
}

bool uros_deinit(void) {
  if (!atomic_exchange(&s_running, false)) return atomic_load(&s_stopped);
  xSemaphoreTake(s_stop_ack, 0);
  if (xTaskNotify(s_mw_uros_task_hd, UROS_NOTIFY_STOP, eSetBits) != pdPASS ||
      xSemaphoreTake(s_stop_ack, pdMS_TO_TICKS(5000)) != pdTRUE) {
    log_error(TAG, "micro-ROS worker stop timed out");
    return false;
  }
  return s_stop_result;
}

void uros_set_domain_id(uint8_t domain_id) {
  entity_set_domain_id((size_t)domain_id);
}

bool uros_is_connected(void) {
  return atomic_load(&s_running) && agent_is_connected();
}

bool uros_sub_set_callback(uros_sub_callback_t cb) {
  return subscriber_set_callback(cb);
}

bool uros_srv_set_callback(uros_srv_callback_t cb) {
  return service_set_callback(cb);
}

bool uros_action_set_callback(uros_action_goal_callback_t cb_goal, uros_action_worker_callback_t cb_worker, uros_action_cancel_callback_t cb_cancel) {
  return action_set_callback(cb_goal, cb_worker, cb_cancel);
}

bool uros_pub(uros_pub_data_flag_t data_flag, uros_pub_data_t *data) {
  if (!atomic_load(&s_running) || mw_uros_task_queue_hd == NULL || data == NULL) return false;
  mw_uros_task_queue_data_t buff;
  buff.data_flag = data_flag;
  memcpy(&buff.data, data, sizeof(uros_pub_data_t));

  if (xQueueSend(mw_uros_task_queue_hd, &buff, 0) != pdTRUE) {
    log_warning(TAG, "Publish queue full!! message dropped!!");

    return false;
  }

  return true;
}
