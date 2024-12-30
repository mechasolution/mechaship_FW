#include <FreeRTOS.h>
#include <task.h>

#include "rtos/peripheral_task/actuator_task.h"
#include "rtos/peripheral_task/lcd_task.h"
#include "rtos/uros_task/uros_task.h"

#include "driver/log/log.h"
#include "driver/power/power.h"

#include "central_task.h"

#define TAG "central"

#define CENTRAL_TASK_SIZE configMINIMAL_STACK_SIZE
TaskHandle_t central_task_hd = NULL;
static StackType_t s_central_task_buff[CENTRAL_TASK_SIZE];
static StaticTask_t s_central_task_struct;

#define CENTRAL_TASK_QUEUE_LENGTH 10
#define CENTRAL_TASK_QUEUE_ITEM_SIZE sizeof(central_task_queue_data_t)
QueueHandle_t central_task_queue_hd = NULL;
static uint8_t s_central_task_queue_buff[CENTRAL_TASK_QUEUE_LENGTH * CENTRAL_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_central_task_queue_struct;

// internal
#define SLED_TASK_SIZE configMINIMAL_STACK_SIZE
static StackType_t s_sled_task_buff[SLED_TASK_SIZE];
static StaticTask_t s_sled_task_struct;

typedef struct {
  enum {
    SLED_PATTERN_NONE = 0,

    SLED_PATTERN_UROS_DISCONNECTED,
    SLED_PATTERN_TURNING_OFF,

    SLED_PATTERN_MAX,
  } pattern;
} sled_task_queue_data_t;
#define SLED_TASK_QUEUE_LENGTH 5
#define SLED_TASK_QUEUE_ITEM_SIZE sizeof(sled_task_queue_data_t)
static QueueHandle_t s_sled_task_queue_hd = NULL;
static uint8_t s_sled_task_queue_buff[SLED_TASK_QUEUE_LENGTH * SLED_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_sled_task_queue_struct;

static void s_power_off(bool is_low_power) {
  log_warning(TAG, "Power off sequence start");

  actuator_task_queue_data_t actuator_queue_data = {0};
  lcd_task_queue_data_t lcd_queue_data = {0};
  uros_task_pub_queue_data_t uros_pub_queue_data = {0};
  sled_task_queue_data_t sled_queue_data = {0};

  power_set_act(false);

  actuator_queue_data.command = ACTUATOR_TASK_COMMAND_TONE;
  actuator_queue_data.data.tone.duration_ms = 3000;
  actuator_queue_data.data.tone.hz = 2000;
  xQueueSend(actuator_task_queue_hd, &actuator_queue_data, portMAX_DELAY);

  sled_queue_data.pattern = SLED_PATTERN_TURNING_OFF;
  xQueueSend(s_sled_task_queue_hd, &sled_queue_data, portMAX_DELAY);

  uros_pub_queue_data.topic = UROS_TASK_PUB_POWER_OFF;
  xQueueSend(uros_task_pub_queue_hd, &uros_pub_queue_data, portMAX_DELAY);

  vTaskDelay(1000 / portTICK_PERIOD_MS); // wait until uros pub done

  vTaskSuspend(uros_task_hd); // stop uros task

  for (int i = 60; i >= 0; i--) {
    log_warning(TAG, "Power down in %d seconds", i);

    lcd_queue_data.command = LCD_TASK_COMMAND_POWER_OFF;
    lcd_queue_data.data.power_off.countdown = i;
    lcd_queue_data.data.power_off.is_low_power = is_low_power;
    xQueueSend(lcd_task_queue_hd, &lcd_queue_data, 0);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  power_set_sbc(false);
  power_set_main(false);
}

static void s_central_task(void *arg) {
  central_task_queue_data_t queue_data = {0};
  lcd_task_queue_data_t lcd_task_queue_data = {0};
  sled_task_queue_data_t sled_task_queue_data = {0};

  for (;;) {
    xQueueReceive(central_task_queue_hd, &queue_data, portMAX_DELAY);
    switch (queue_data.event_type) {
    case CENTRAL_TASK_EVENT_POWER_OFF:
      s_power_off(false);
      break;

    case CENTRAL_TASK_EVENT_UROS_CONNECTION:
      lcd_task_queue_data.command = LCD_TASK_COMMAND_CONNECTION;
      lcd_task_queue_data.data.connection.status = queue_data.data.uros_connection.status;
      xQueueSend(lcd_task_queue_hd, &lcd_task_queue_data, 0);

      sled_task_queue_data.pattern = queue_data.data.uros_connection.status ? SLED_PATTERN_NONE : SLED_PATTERN_UROS_DISCONNECTED;
      xQueueSend(s_sled_task_queue_hd, &sled_task_queue_data, 0);

      if (queue_data.data.uros_connection.status == false) {
        actuator_task_queue_data_t temp;
        temp.command = ACTUATOR_TASK_COMMAND_POWER;
        temp.data.power.power_target = false;
        xQueueSend(actuator_task_queue_hd, &temp, 0);
      }
      break;

    case CENTRAL_TASK_EVENT_LOW_POWER:
      s_power_off(true);
      break;

    case CENTRAL_TASK_EVENT_NONE:
    default:
      break;
    }
  }
}

// Status LED 패턴 구현
/*
  -____-____ -> SBC 부팅중
  --__--__-- -> micro-ROS 연결 대기
  ---------- -> 일반 상태
  -_-_-_-_-_ -> 종료중
*/
static void s_sled_task(void *arg) {
  actuator_task_queue_data_t actuator_task_queue_data;
  sled_task_queue_data_t queue_data;
  uint8_t pattern_idx = 0;
  bool pattern[SLED_PATTERN_MAX][4] = {
      {true, true, true, true},   // SLED_PATTERN_NONE
      {true, true, false, false}, // SLED_PATTERN_UROS_DISCONNECTED
      {true, false, true, false}, // SLED_PATTERN_TURNING_OFF
  };

  queue_data.pattern = SLED_PATTERN_UROS_DISCONNECTED;
  actuator_task_queue_data.command = ACTUATOR_TASK_COMMAND_LED_S;

  for (;;) {
    if (xQueueReceive(s_sled_task_queue_hd, &queue_data, 500 / portTICK_PERIOD_MS) == pdTRUE) {
      if (queue_data.pattern >= SLED_PATTERN_MAX || queue_data.pattern <= SLED_PATTERN_NONE) {
        queue_data.pattern = SLED_PATTERN_NONE;
      }
    }

    actuator_task_queue_data.data.led_s.value = pattern[queue_data.pattern][pattern_idx];
    xQueueSend(actuator_task_queue_hd, &actuator_task_queue_data, 0);

    pattern_idx = pattern_idx >= 3 ? 0 : pattern_idx + 1;
  }
}

bool central_task_init(void) {
  central_task_hd = xTaskCreateStatic(
      s_central_task,
      "central",
      CENTRAL_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES,
      s_central_task_buff,
      &s_central_task_struct);

  xTaskCreateStatic(
      s_sled_task,
      "sled",
      SLED_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES,
      s_sled_task_buff,
      &s_sled_task_struct);
}

bool central_task_queue_init(void) {
  central_task_queue_hd = xQueueCreateStatic(
      CENTRAL_TASK_QUEUE_LENGTH,
      CENTRAL_TASK_QUEUE_ITEM_SIZE,
      s_central_task_queue_buff,
      &s_central_task_queue_struct);

  s_sled_task_queue_hd = xQueueCreateStatic(
      SLED_TASK_QUEUE_LENGTH,
      SLED_TASK_QUEUE_ITEM_SIZE,
      s_sled_task_queue_buff,
      &s_sled_task_queue_struct);
}