#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "driver/actuator/actuator.h"
#include "driver/led/led.h"
#include "driver/power/power.h"
#include "driver/rgbw_led/rgbw_led.h"
#include "driver/tone/tone.h"

#include "actuator_task.h"
#include "lcd_task.h"

#define ACTUATOR_TASK_SIZE 512
TaskHandle_t actuator_task_hd;
static StackType_t s_actuator_task_buff[ACTUATOR_TASK_SIZE];
static StaticTask_t s_actuator_task_struct;

#define ACTUATOR_TASK_QUEUE_LENGTH 10
#define ACTUATOR_TASK_QUEUE_ITEM_SIZE sizeof(actuator_task_queue_data_t)
QueueHandle_t actuator_task_queue_hd;
static uint8_t s_actuator_queue_buff[ACTUATOR_TASK_QUEUE_LENGTH * ACTUATOR_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_actuator_queue_struct;

// internal
#define TONE_TASK_SIZE configMINIMAL_STACK_SIZE
static StackType_t s_tone_task_buff[TONE_TASK_SIZE];
static StaticTask_t s_tone_task_struct;

typedef struct {
  uint16_t hz;
  uint16_t duration_ms;
} tone_task_queue_data_t;
#define TONE_TASK_QUEUE_LENGTH 10
#define TONE_TASK_QUEUE_ITEM_SIZE sizeof(tone_task_queue_data_t)
static QueueHandle_t tone_task_queue_hd = NULL;
static uint8_t s_tone_task_queue_buff[TONE_TASK_QUEUE_LENGTH * TONE_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_tone_task_queue_struct;

static void s_actuator_task(void *arg) {
  (void)arg;

  actuator_task_queue_data_t queue_data = {0};
  lcd_task_queue_data_t lcd_queue_data;
  tone_task_queue_data_t tone_queue_data;

  for (;;) {
    if (xQueueReceive(actuator_task_queue_hd, &queue_data, portMAX_DELAY) == pdTRUE) {
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

      case ACTUATOR_TASK_COMMAND_LED_1:
        led_set_1(queue_data.data.led_1.value);
        break;

      case ACTUATOR_TASK_COMMAND_LED_2:
        led_set_2(queue_data.data.led_2.value);
        break;

      case ACTUATOR_TASK_COMMAND_LED_S:
        led_set_s(queue_data.data.led_s.value);
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

        if (queue_data.data.power.power_target) {
          lcd_queue_data.command = LCD_TASK_COMMAND_ACT_POWER;
          lcd_queue_data.data.act_power.status = true;
          xQueueSend(lcd_task_queue_hd, &lcd_queue_data, 0);

          tone_queue_data.duration_ms = 150;
          tone_queue_data.hz = 523;
          xQueueSend(tone_task_queue_hd, &tone_queue_data, 0);
          tone_queue_data.hz = 622;
          xQueueSend(tone_task_queue_hd, &tone_queue_data, 0);
          tone_queue_data.hz = 739;
          xQueueSend(tone_task_queue_hd, &tone_queue_data, 0);

          if (queue_data.data.power.key_pulse_180_degree == 0 && queue_data.data.power.thruster_pulse_100_percentage == 0) {
            actuator_set_key_info(500,
                                  2500,
                                  0,
                                  180);
            actuator_set_thruster_info(1500,
                                       2000);
          } else {
            actuator_set_key_info(queue_data.data.power.key_pulse_0_degree,
                                  queue_data.data.power.key_pulse_180_degree,
                                  queue_data.data.power.key_min_degree,
                                  queue_data.data.power.key_max_degree);
            actuator_set_thruster_info(queue_data.data.power.thruster_pulse_0_percentage,
                                       queue_data.data.power.thruster_pulse_100_percentage);
          }

          actuator_set_key_degree(90);
          actuator_set_thruster_percentage(0);

          power_set_act(true);
          vTaskDelay(5000 / portTICK_PERIOD_MS); // wait until esc init finished
        } else {
          lcd_queue_data.command = LCD_TASK_COMMAND_ACT_POWER;
          lcd_queue_data.data.act_power.status = false;
          xQueueSend(lcd_task_queue_hd, &lcd_queue_data, 0);

          tone_queue_data.duration_ms = 150;
          tone_queue_data.hz = 739;
          xQueueSend(tone_task_queue_hd, &tone_queue_data, 0);
          tone_queue_data.hz = 622;
          xQueueSend(tone_task_queue_hd, &tone_queue_data, 0);
          tone_queue_data.hz = 523;
          xQueueSend(tone_task_queue_hd, &tone_queue_data, 0);

          power_set_act(false);
          actuator_pwm_off();
        }
        break;

      case ACTUATOR_TASK_COMMAND_TONE:
        tone_queue_data.duration_ms = queue_data.data.tone.duration_ms;
        tone_queue_data.hz = queue_data.data.tone.hz;
        xQueueSend(tone_task_queue_hd, &tone_queue_data, 0);
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
    if (xQueueReceive(tone_task_queue_hd, &queue_data, portMAX_DELAY) == pdTRUE) {
      tone_set(queue_data.hz);
      vTaskDelay(queue_data.duration_ms / portTICK_PERIOD_MS);
      tone_reset();
    }
  }
}

bool actuator_task_init(void) {
  actuator_task_hd = xTaskCreateStatic(
      s_actuator_task,
      "actuator",
      ACTUATOR_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES,
      s_actuator_task_buff,
      &s_actuator_task_struct);

  xTaskCreateStatic(
      s_tone_task,
      "tone",
      TONE_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES,
      s_tone_task_buff,
      &s_tone_task_struct);

  return true;
}

bool actuator_task_queue_init(void) {
  actuator_task_queue_hd = xQueueCreateStatic(
      ACTUATOR_TASK_QUEUE_LENGTH,
      ACTUATOR_TASK_QUEUE_ITEM_SIZE,
      s_actuator_queue_buff,
      &s_actuator_queue_struct);

  tone_task_queue_hd = xQueueCreateStatic(
      TONE_TASK_QUEUE_LENGTH,
      TONE_TASK_QUEUE_ITEM_SIZE,
      s_tone_task_queue_buff,
      &s_tone_task_queue_struct);

  return true;
}
