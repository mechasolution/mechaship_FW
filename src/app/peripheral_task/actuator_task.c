#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "driver/actuator/actuator.h"
#include "driver/log/log.h"
#include "driver/power/power.h"
#include "driver/rgbw_led/rgbw_led.h"
#include "driver/tone/tone.h"

#include "actuator_task.h"
#include "lcd_task.h"

#define TAG "actuator"

typedef struct {
  enum {
    ACTUATOR_TASK_COMMAND_NONE = 0x00,

    ACTUATOR_TASK_COMMAND_THROTTLE,
    ACTUATOR_TASK_COMMAND_KEY,
    ACTUATOR_TASK_COMMAND_RGBWLED,
    ACTUATOR_TASK_COMMAND_POWER,
    ACTUATOR_TASK_COMMAND_TONE,
  } command;
  union {
    struct { // ACTUATOR_TASK_COMMAND_THROTTLE
      float percentage;
    } throttle;

    struct { // ACTUATOR_TASK_COMMAND_KEY
      float degree;
    } key;

    struct { // ACTUATOR_TASK_COMMAND_RGBWLED
      uint8_t red;
      uint8_t green;
      uint8_t blue;
      uint8_t white;
    } rgbwled;

    struct { // ACTUATOR_TASK_COMMAND_POWER
      bool power_target;

      float key_min_degree; // 키 최소 각도
      float key_max_degree; // 키 최대 각도

      uint16_t key_pulse_0_degree;   // 키 최소 각도 펄스
      uint16_t key_pulse_180_degree; // 키 최대 각도 펄스

      uint16_t thruster_pulse_0_percentage;   // ESC 중립 펄스
      uint16_t thruster_pulse_100_percentage; // ESC 최대속도 펄스
    } power;

    struct { // ACTUATOR_TASK_COMMAND_TONE
      uint16_t hz;
      uint16_t duration_ms;
    } tone;
  } data;

} actuator_task_queue_data_t;
#define ACTUATOR_TASK_QUEUE_LENGTH 20
#define ACTUATOR_TASK_QUEUE_ITEM_SIZE sizeof(actuator_task_queue_data_t)
static QueueHandle_t s_actuator_task_queue_hd;
static uint8_t s_actuator_queue_buff[ACTUATOR_TASK_QUEUE_LENGTH * ACTUATOR_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_actuator_queue_struct;

#define ACTUATOR_TASK_SIZE 512
static StackType_t s_actuator_task_buff[ACTUATOR_TASK_SIZE];
static StaticTask_t s_actuator_task_struct;

#define TONE_TASK_SIZE configMINIMAL_STACK_SIZE
static StackType_t s_tone_task_buff[TONE_TASK_SIZE];
static StaticTask_t s_tone_task_struct;

typedef struct {
  uint16_t hz;
  uint16_t duration_ms;
} tone_task_queue_data_t;
#define TONE_TASK_QUEUE_LENGTH 20
#define TONE_TASK_QUEUE_ITEM_SIZE sizeof(tone_task_queue_data_t)
static QueueHandle_t s_tone_task_queue_hd = NULL;
static uint8_t s_tone_task_queue_buff[TONE_TASK_QUEUE_LENGTH * TONE_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_tone_task_queue_struct;

static void s_actuator_task(void *arg) {
  (void)arg;

  actuator_task_queue_data_t queue_data = {0};
  tone_task_queue_data_t tone_queue_data;

  for (;;) {
    if (xQueueReceive(s_actuator_task_queue_hd, &queue_data, portMAX_DELAY) == pdTRUE) {
      switch (queue_data.command) {
      case ACTUATOR_TASK_COMMAND_THROTTLE:
        if (power_get_act()) {
          actuator_set_thruster_percentage(queue_data.data.throttle.percentage);
        }
        break;

      case ACTUATOR_TASK_COMMAND_KEY:
        if (power_get_act()) {
          actuator_set_key_degree(queue_data.data.key.degree);
        }
        break;

      case ACTUATOR_TASK_COMMAND_RGBWLED:
        rgbw_led_set_pixels(rgbw_led_get_color(
            queue_data.data.rgbwled.red,
            queue_data.data.rgbwled.green,
            queue_data.data.rgbwled.blue,
            queue_data.data.rgbwled.white));
        break;

      case ACTUATOR_TASK_COMMAND_POWER:
        if (power_get_act() == queue_data.data.power.power_target) {
          break;
        }

        lcd_task_update_actuator_power(queue_data.data.power.power_target);
        if (queue_data.data.power.power_target == true) {
          lcd_task_update_key(90);
          lcd_task_update_throttle(0);
          if (queue_data.data.power.key_pulse_180_degree == 0 && queue_data.data.power.thruster_pulse_100_percentage == 0) {
            // if data is all zero, set test value (for debug)
            queue_data.data.power.key_min_degree = 0;
            queue_data.data.power.key_max_degree = 180;
            queue_data.data.power.key_pulse_0_degree = 500;
            queue_data.data.power.key_pulse_180_degree = 2500;
            queue_data.data.power.thruster_pulse_0_percentage = 1500;
            queue_data.data.power.thruster_pulse_100_percentage = 1900;
          }

          actuator_set_key_info(queue_data.data.power.key_pulse_0_degree,
                                queue_data.data.power.key_pulse_180_degree,
                                queue_data.data.power.key_min_degree,
                                queue_data.data.power.key_max_degree);
          actuator_set_thruster_info(queue_data.data.power.thruster_pulse_0_percentage,
                                     queue_data.data.power.thruster_pulse_100_percentage);

          actuator_set_key_degree(90);
          actuator_set_thruster_percentage(0);

          power_set_act(true);
        } else {
          power_set_act(false);
          actuator_pwm_off();
        }
        break;

      case ACTUATOR_TASK_COMMAND_TONE:
        tone_queue_data.duration_ms = queue_data.data.tone.duration_ms;
        tone_queue_data.hz = queue_data.data.tone.hz;
        xQueueSend(s_tone_task_queue_hd, &tone_queue_data, 0);
        break;

      case ACTUATOR_TASK_COMMAND_NONE:
      default:
        break;
      }
    }
  }
}

static void s_tone_task(void *arg) {
  (void)arg;

  tone_task_queue_data_t queue_data = {0};

  for (;;) {
    if (xQueueReceive(s_tone_task_queue_hd, &queue_data, portMAX_DELAY) == pdTRUE) {
      tone_set(queue_data.hz);
      vTaskDelay(queue_data.duration_ms / portTICK_PERIOD_MS);

      if (uxQueueMessagesWaiting(s_tone_task_queue_hd) == 0) {
        tone_reset();
      }
    }
  }
}

bool actuator_task_init(void) {
  s_actuator_task_queue_hd = xQueueCreateStatic(
      ACTUATOR_TASK_QUEUE_LENGTH,
      ACTUATOR_TASK_QUEUE_ITEM_SIZE,
      s_actuator_queue_buff,
      &s_actuator_queue_struct);

  s_tone_task_queue_hd = xQueueCreateStatic(
      TONE_TASK_QUEUE_LENGTH,
      TONE_TASK_QUEUE_ITEM_SIZE,
      s_tone_task_queue_buff,
      &s_tone_task_queue_struct);

  xTaskCreateStatic(
      s_actuator_task,
      "actuator",
      ACTUATOR_TASK_SIZE,
      NULL,
      configEVENT_TASK_PRIORITIES,
      s_actuator_task_buff,
      &s_actuator_task_struct);

  xTaskCreateStatic(
      s_tone_task,
      "tone",
      TONE_TASK_SIZE,
      NULL,
      configEVENT_TASK_PRIORITIES,
      s_tone_task_buff,
      &s_tone_task_struct);

  return true;
}

static bool s_send_queue(actuator_task_queue_data_t *queue_data) {
  bool ret = xQueueSend(s_actuator_task_queue_hd, queue_data, 0) == pdTRUE;
  if (ret == false) {
    log_warning(TAG, "Publish queue full!! message dropped!!");
  }

  return ret;
}

bool actuator_task_set_throttle(float percentage) {
  actuator_task_queue_data_t queue_data;
  queue_data.command = ACTUATOR_TASK_COMMAND_THROTTLE;
  queue_data.data.throttle.percentage = percentage;

  return s_send_queue(&queue_data);
}

bool actuator_task_set_key(float degree) {
  actuator_task_queue_data_t queue_data;
  queue_data.command = ACTUATOR_TASK_COMMAND_KEY;
  queue_data.data.key.degree = degree;

  return s_send_queue(&queue_data);
}

bool actuator_task_set_rgbwled(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
  actuator_task_queue_data_t queue_data;
  queue_data.command = ACTUATOR_TASK_COMMAND_RGBWLED;
  queue_data.data.rgbwled.red = red;
  queue_data.data.rgbwled.green = green;
  queue_data.data.rgbwled.blue = blue;
  queue_data.data.rgbwled.white = white;

  return s_send_queue(&queue_data);
}

bool actuator_task_set_power(bool power_target,
                             float key_min_degree,
                             float key_max_degree,
                             uint16_t key_pulse_0_degree,
                             uint16_t key_pulse_180_degree,
                             uint16_t thruster_pulse_0_percentage,
                             uint16_t thruster_pulse_100_percentage) {
  actuator_task_queue_data_t queue_data;
  queue_data.command = ACTUATOR_TASK_COMMAND_POWER;
  queue_data.data.power.power_target = power_target;
  queue_data.data.power.key_min_degree = key_min_degree;
  queue_data.data.power.key_max_degree = key_max_degree;
  queue_data.data.power.key_pulse_0_degree = key_pulse_0_degree;
  queue_data.data.power.key_pulse_180_degree = key_pulse_180_degree;
  queue_data.data.power.thruster_pulse_0_percentage = thruster_pulse_0_percentage;
  queue_data.data.power.thruster_pulse_100_percentage = thruster_pulse_100_percentage;

  return s_send_queue(&queue_data);
}

bool actuator_task_set_tone(uint16_t hz, uint16_t duration_ms) {
  actuator_task_queue_data_t queue_data;
  queue_data.command = ACTUATOR_TASK_COMMAND_TONE;
  queue_data.data.tone.hz = hz;
  queue_data.data.tone.duration_ms = duration_ms;

  return s_send_queue(&queue_data);
}
