#include <pico/stdlib.h>
#include <stdio.h>
#include <time.h>

#include <FreeRTOS.h>
#include <queue.h>
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
QueueHandle_t mw_uros_task_queue_hd = NULL;
static uint8_t s_mw_uros_task_queue_buff[MW_UROS_TASK_QUEUE_LENGTH * MW_UROS_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_mw_uros_task_queue_struct;

#define MW_UROS_TASK_SIZE 4096
static TaskHandle_t s_mw_uros_task_hd = NULL;
static StackType_t s_mw_uros_task_buff[MW_UROS_TASK_SIZE];
static StaticTask_t s_mw_uros_task_struct;
static void s_uros_task(void *arg) {
  (void)arg;

  mw_uros_task_queue_data_t queue_buff;

  agent_init();

  for (;;) {
    agent_spin();
    while (agent_is_connected() && xQueueReceive(mw_uros_task_queue_hd, &queue_buff, 0) == pdTRUE) {
      bool ret = publisher_publish(queue_buff.data_flag, &queue_buff.data);
      if (ret == false) {
        if (uros_is_connected()) {
          entity_destroy();
        }
        agent_reset();
      }
    }
  }
}

bool uros_init(void) {
  uint8_t cnt = 0;

  if (mw_uros_task_queue_hd == NULL) {
    mw_uros_task_queue_hd = xQueueCreateStatic(
        MW_UROS_TASK_QUEUE_LENGTH,
        MW_UROS_TASK_QUEUE_ITEM_SIZE,
        s_mw_uros_task_queue_buff,
        &s_mw_uros_task_queue_struct);
    cnt++;
  }

  if (s_mw_uros_task_hd == NULL) {
    s_mw_uros_task_hd = xTaskCreateStatic(
        s_uros_task,
        "uros",
        MW_UROS_TASK_SIZE,
        NULL,
        configMAX_PRIORITIES - 2,
        s_mw_uros_task_buff,
        &s_mw_uros_task_struct);
    cnt++;
  }

  return cnt == 2;
}

bool uros_deinit(void) {
  uint8_t cnt = 0;

  if (s_mw_uros_task_hd != NULL) {
    vTaskDelete(s_mw_uros_task_hd);
    s_mw_uros_task_hd = NULL;
    cnt++;
  }

  if (mw_uros_task_queue_hd != NULL) {
    vQueueDelete(mw_uros_task_queue_hd);
    mw_uros_task_queue_hd = NULL;
    cnt++;
  }

  if (cnt != 2) {
    return false;
  }

  if (uros_is_connected()) {
    entity_destroy();
  }
  agent_reset();

  return true;
}

void uros_set_domain_id(uint8_t domain_id) {
  entity_set_domain_id((size_t)domain_id);
}

bool uros_is_connected(void) {
  return agent_is_connected();
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
  mw_uros_task_queue_data_t buff;
  buff.data_flag = data_flag;
  memcpy(&buff.data, data, sizeof(uros_pub_data_t));

  if (xQueueSend(mw_uros_task_queue_hd, &buff, 0) != pdTRUE) {
    log_warning(TAG, "Publish queue full!! message dropped!!");

    return false;
  }

  return true;
}