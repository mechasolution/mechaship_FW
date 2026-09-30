#include <math.h>
#include <stdatomic.h>
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

#include "driver/actuator/actuator.h"
#include "driver/log/log.h"
#include "driver/power/power.h"
#include "driver/rgbw_led/rgbw_led.h"
#include "driver/tone/tone.h"

#include "actuator_task.h"
#include "lcd_task.h"

#define TAG "actuator"

#ifdef MECHASHIP_ESC_PROTECTION
#define ESC_PROTECTION_DURATION_MS 5000
#define ESC_PROTECTION_BLINK_HALF_PERIOD_MS 125
#define ESC_PROTECTION_COLOR rgbw_led_get_color(180, 55, 0, 0)
#endif

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
    struct { // ACTUATOR_TASK_COMMAND_RGBWLED
      uint8_t red;
      uint8_t green;
      uint8_t blue;
      uint8_t white;
    } rgbwled;

    struct { // ACTUATOR_TASK_COMMAND_POWER
      uint32_t request_id;
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
_Static_assert(sizeof(float) == sizeof(uint32_t), "motion value must fit in 32 bits");
static atomic_uint s_latest_throttle_bits;
static atomic_uint s_latest_key_bits;
static atomic_bool s_throttle_pending;
static atomic_bool s_key_pending;
#ifdef MECHASHIP_ESC_PROTECTION
static atomic_bool s_power_off_pending;
#endif
typedef struct {
  uint32_t request_id;
  bool applied;
} power_ack_t;
static QueueHandle_t s_power_ack_queue;
static uint8_t s_power_ack_storage[sizeof(power_ack_t)];
static StaticQueue_t s_power_ack_queue_struct;
static SemaphoreHandle_t s_power_request_mutex;
static StaticSemaphore_t s_power_request_mutex_struct;
static uint32_t s_next_power_request_id;

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

#ifdef MECHASHIP_ESC_PROTECTION
  bool esc_protection_active = false;
  bool protection_led_on = false;
  TickType_t protection_start_tick = 0;
  TickType_t last_blink_tick = 0;
  float requested_throttle = 0.0f;
  float requested_key = 90.0f;
  rgbw_color_data_t requested_led_color = 0;
#endif

  for (;;) {
    TickType_t receive_timeout = portMAX_DELAY;
#ifdef MECHASHIP_ESC_PROTECTION
    if (esc_protection_active) {
      TickType_t now = xTaskGetTickCount();
      TickType_t elapsed = now - protection_start_tick;
      if (elapsed >= pdMS_TO_TICKS(ESC_PROTECTION_DURATION_MS)) {
        esc_protection_active = false;
        if (atomic_load(&s_throttle_pending)) {
          uint32_t bits = atomic_load(&s_latest_throttle_bits);
          memcpy(&requested_throttle, &bits, sizeof(requested_throttle));
        }
        if (atomic_load(&s_key_pending)) {
          uint32_t bits = atomic_load(&s_latest_key_bits);
          memcpy(&requested_key, &bits, sizeof(requested_key));
        }
        if (power_get_act() && !atomic_load(&s_power_off_pending)) {
          actuator_set_thruster_percentage(requested_throttle);
          actuator_set_key_degree(requested_key);
        }
        rgbw_led_set_pixels(requested_led_color);
      } else {
        TickType_t blink_elapsed = now - last_blink_tick;
        if (blink_elapsed >= pdMS_TO_TICKS(ESC_PROTECTION_BLINK_HALF_PERIOD_MS)) {
          protection_led_on = !protection_led_on;
          rgbw_led_set_pixels(protection_led_on ? ESC_PROTECTION_COLOR : 0);
          last_blink_tick = now;
          blink_elapsed = 0;
        }
        TickType_t until_end = pdMS_TO_TICKS(ESC_PROTECTION_DURATION_MS) - elapsed;
        TickType_t until_blink = pdMS_TO_TICKS(ESC_PROTECTION_BLINK_HALF_PERIOD_MS) - blink_elapsed;
        receive_timeout = until_end < until_blink ? until_end : until_blink;
      }
    }
#endif
    if (xQueueReceive(s_actuator_task_queue_hd, &queue_data, receive_timeout) == pdTRUE) {
      switch (queue_data.command) {
      case ACTUATOR_TASK_COMMAND_THROTTLE:
        atomic_store(&s_throttle_pending, false);
        if (power_get_act()) {
          uint32_t bits = atomic_load(&s_latest_throttle_bits);
          float percentage;
          memcpy(&percentage, &bits, sizeof(percentage));
#ifdef MECHASHIP_ESC_PROTECTION
          if (esc_protection_active) {
            requested_throttle = percentage;
          } else {
            actuator_set_thruster_percentage(percentage);
          }
#else
          actuator_set_thruster_percentage(percentage);
#endif
        }
        break;

      case ACTUATOR_TASK_COMMAND_KEY:
        atomic_store(&s_key_pending, false);
        if (power_get_act()) {
          uint32_t bits = atomic_load(&s_latest_key_bits);
          float degree;
          memcpy(&degree, &bits, sizeof(degree));
#ifdef MECHASHIP_ESC_PROTECTION
          if (esc_protection_active) {
            requested_key = degree;
          } else {
            actuator_set_key_degree(degree);
          }
#else
          actuator_set_key_degree(degree);
#endif
        }
        break;

      case ACTUATOR_TASK_COMMAND_RGBWLED: {
        rgbw_color_data_t color = rgbw_led_get_color(
            queue_data.data.rgbwled.red,
            queue_data.data.rgbwled.green,
            queue_data.data.rgbwled.blue,
            queue_data.data.rgbwled.white);
#ifdef MECHASHIP_ESC_PROTECTION
        requested_led_color = color;
        if (!esc_protection_active) rgbw_led_set_pixels(color);
#else
        rgbw_led_set_pixels(color);
#endif
        break;
      }

      case ACTUATOR_TASK_COMMAND_POWER: {
        bool target = queue_data.data.power.power_target;
        bool was_on = power_get_act();
        if (was_on != target || target) {
          lcd_task_update_actuator_power(target);
          if (target) {
#ifdef MECHASHIP_ESC_PROTECTION
            if (!was_on) {
#endif
              lcd_task_update_key(90);
              lcd_task_update_throttle(0);
              if (queue_data.data.power.key_pulse_180_degree == 0 && queue_data.data.power.thruster_pulse_100_percentage == 0) {
                // Existing bench-test defaults for an empty calibration request.
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
#ifdef MECHASHIP_ESC_PROTECTION
              requested_throttle = 0.0f;
              requested_key = 90.0f;
              esc_protection_active = true;
              protection_start_tick = xTaskGetTickCount();
              last_blink_tick = protection_start_tick;
              protection_led_on = true;
              rgbw_led_set_pixels(ESC_PROTECTION_COLOR);
            }
#endif
          } else {
            power_set_act(false);
            actuator_pwm_off();
#ifdef MECHASHIP_ESC_PROTECTION
            if (esc_protection_active) {
              esc_protection_active = false;
              rgbw_led_set_pixels(requested_led_color);
            }
#endif
          }
        }
#ifdef MECHASHIP_ESC_PROTECTION
        if (!target) atomic_store(&s_power_off_pending, false);
#endif
        power_ack_t ack = {queue_data.data.power.request_id, power_get_act() == target};
        xQueueOverwrite(s_power_ack_queue, &ack);
        break;
      }

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
  s_power_ack_queue = xQueueCreateStatic(
      1, sizeof(power_ack_t), s_power_ack_storage, &s_power_ack_queue_struct);
  s_power_request_mutex = xSemaphoreCreateMutexStatic(&s_power_request_mutex_struct);

  TaskHandle_t actuator_task = xTaskCreateStatic(
      s_actuator_task,
      "actuator",
      ACTUATOR_TASK_SIZE,
      NULL,
      configEVENT_TASK_PRIORITIES,
      s_actuator_task_buff,
      &s_actuator_task_struct);

  TaskHandle_t tone_task = xTaskCreateStatic(
      s_tone_task,
      "tone",
      TONE_TASK_SIZE,
      NULL,
      configEVENT_TASK_PRIORITIES,
      s_tone_task_buff,
      &s_tone_task_struct);

  return s_actuator_task_queue_hd != NULL && s_tone_task_queue_hd != NULL &&
         s_power_ack_queue != NULL && s_power_request_mutex != NULL &&
         actuator_task != NULL && tone_task != NULL;
}

static bool s_send_queue(actuator_task_queue_data_t *queue_data) {
  if (s_actuator_task_queue_hd == NULL) return false;
  bool ret = xQueueSend(s_actuator_task_queue_hd, queue_data, 0) == pdTRUE;
  if (ret == false) {
    log_warning(TAG, "Publish queue full!! message dropped!!");
  }

  return ret;
}

bool actuator_task_set_throttle(float percentage) {
  if (!isfinite(percentage)) return false;
  uint32_t bits;
  memcpy(&bits, &percentage, sizeof(bits));
  atomic_store(&s_latest_throttle_bits, bits);
  if (atomic_exchange(&s_throttle_pending, true)) return true;
  actuator_task_queue_data_t queue_data;
  queue_data.command = ACTUATOR_TASK_COMMAND_THROTTLE;
  if (s_send_queue(&queue_data)) return true;
  atomic_store(&s_throttle_pending, false);
  return false;
}

bool actuator_task_set_key(float degree) {
  if (!isfinite(degree)) return false;
  uint32_t bits;
  memcpy(&bits, &degree, sizeof(bits));
  atomic_store(&s_latest_key_bits, bits);
  if (atomic_exchange(&s_key_pending, true)) return true;
  actuator_task_queue_data_t queue_data;
  queue_data.command = ACTUATOR_TASK_COMMAND_KEY;
  if (s_send_queue(&queue_data)) return true;
  atomic_store(&s_key_pending, false);
  return false;
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
  if (s_actuator_task_queue_hd == NULL || s_power_ack_queue == NULL || s_power_request_mutex == NULL) return false;
  if (power_target && (key_pulse_180_degree != 0 || thruster_pulse_100_percentage != 0)) {
    if (!isfinite(key_min_degree) || !isfinite(key_max_degree) ||
        key_min_degree < 0 || key_max_degree > 180 || key_min_degree > key_max_degree ||
        key_pulse_0_degree > 20000 || key_pulse_180_degree > 20000 ||
        thruster_pulse_0_percentage > 20000 || thruster_pulse_100_percentage > 20000) {
      return false;
    }
  }
  if (xSemaphoreTake(s_power_request_mutex, pdMS_TO_TICKS(1500)) != pdTRUE) return false;
  actuator_task_queue_data_t queue_data = {0};
  queue_data.command = ACTUATOR_TASK_COMMAND_POWER;
  queue_data.data.power.request_id = ++s_next_power_request_id;
  queue_data.data.power.power_target = power_target;
  queue_data.data.power.key_min_degree = key_min_degree;
  queue_data.data.power.key_max_degree = key_max_degree;
  queue_data.data.power.key_pulse_0_degree = key_pulse_0_degree;
  queue_data.data.power.key_pulse_180_degree = key_pulse_180_degree;
  queue_data.data.power.thruster_pulse_0_percentage = thruster_pulse_0_percentage;
  queue_data.data.power.thruster_pulse_100_percentage = thruster_pulse_100_percentage;

  bool queued;
  xQueueReset(s_power_ack_queue);
  if (!power_target) {
#ifdef MECHASHIP_ESC_PROTECTION
    atomic_store(&s_power_off_pending, true);
#endif
    // Old motion commands must not delay or follow a power-off request.
    xQueueReset(s_actuator_task_queue_hd);
    atomic_store(&s_throttle_pending, false);
    atomic_store(&s_key_pending, false);
    queued = xQueueSendToFront(s_actuator_task_queue_hd, &queue_data, 0) == pdTRUE;
  } else {
    queued = s_send_queue(&queue_data);
  }
  if (!queued) {
#ifdef MECHASHIP_ESC_PROTECTION
    if (!power_target) atomic_store(&s_power_off_pending, false);
#endif
    xSemaphoreGive(s_power_request_mutex);
    return false;
  }

  const TickType_t wait_ticks = pdMS_TO_TICKS(1000);
  TickType_t start = xTaskGetTickCount();
  power_ack_t ack;
  bool applied = false;
  while (xTaskGetTickCount() - start < wait_ticks) {
    TickType_t remaining = wait_ticks - (xTaskGetTickCount() - start);
    if (xQueueReceive(s_power_ack_queue, &ack, remaining) == pdTRUE &&
        ack.request_id == queue_data.data.power.request_id) {
      applied = ack.applied;
      break;
    }
  }
  xSemaphoreGive(s_power_request_mutex);
  return applied;
}

bool actuator_task_set_tone(uint16_t hz, uint16_t duration_ms) {
  actuator_task_queue_data_t queue_data;
  queue_data.command = ACTUATOR_TASK_COMMAND_TONE;
  queue_data.data.tone.hz = hz;
  queue_data.data.tone.duration_ms = duration_ms;

  return s_send_queue(&queue_data);
}
