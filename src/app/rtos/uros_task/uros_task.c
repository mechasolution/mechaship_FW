#include <math.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "rtos/central_task/central_task.h"
#include "rtos/peripheral_task/actuator_task.h"
#include "rtos/peripheral_task/lcd_task.h"

#include "driver/log/log.h"
#include "driver/switch8/switch8.h"

#include "hal/time/time.h"

#include "uros/uros.h"

#include "uros_task.h"

#define TAG "uros_task"

#define UROS_TASK_SIZE 4096
TaskHandle_t uros_task_hd = NULL;
static StackType_t s_uros_task_buff[UROS_TASK_SIZE];
static StaticTask_t s_uros_task_struct;

#define UROS_TASK_PUB_QUEUE_LENGTH 10
#define UROS_TASK_PUB_QUEUE_ITEM_SIZE sizeof(uros_task_pub_queue_data_t)
QueueHandle_t uros_task_pub_queue_hd = NULL;
static uint8_t s_uros_task_pub_queue_buff[UROS_TASK_PUB_QUEUE_LENGTH * UROS_TASK_PUB_QUEUE_ITEM_SIZE];
static StaticQueue_t s_uros_task_pub_queue_struct;

static lcd_task_queue_data_t s_lcd_queue_data;
static actuator_task_queue_data_t s_actuator_queue_data;

static uint32_t s_last_ip = 0;

static void s_uros_sub_cb(
    const uros_sub_data_flag_t data_flag,
    const uros_sub_data_t *data) {
  switch (data_flag) {
  case UROS_SUB_KEY:
    s_actuator_queue_data.command = ACTUATOR_TASK_COMMAND_KEY;
    s_actuator_queue_data.data.key.degree = data->key_degree.degree;
    xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, 0);
    break;

  case UROS_SUB_THROTTLE:
    s_actuator_queue_data.command = ACTUATOR_TASK_COMMAND_THROTTLE;
    s_actuator_queue_data.data.throttle.percentage = data->throttle_percentage.percentage;
    xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, 0);
    break;

  case UROS_SUB_LED_USER_1:
    s_actuator_queue_data.command = ACTUATOR_TASK_COMMAND_LED_1;
    s_actuator_queue_data.data.led_1.value = data->led_user_1.value;
    xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, 0);
    break;

  case UROS_SUB_LED_USER_2:
    s_actuator_queue_data.command = ACTUATOR_TASK_COMMAND_LED_2;
    s_actuator_queue_data.data.led_2.value = data->led_user_2.value;
    xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, 0);
    break;

  case UROS_SUB_TONE:
    s_actuator_queue_data.command = ACTUATOR_TASK_COMMAND_TONE;
    s_actuator_queue_data.data.tone.hz = data->tone.hz;
    s_actuator_queue_data.data.tone.duration_ms = data->tone.duration_ms;
    xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, 0);
    break;

  case UROS_SUB_RGBW_LED:
    s_actuator_queue_data.command = ACTUATOR_TASK_COMMAND_RGBWLED;
    s_actuator_queue_data.data.rgbwled.red = data->rgbw_led.red;
    s_actuator_queue_data.data.rgbwled.green = data->rgbw_led.green;
    s_actuator_queue_data.data.rgbwled.blue = data->rgbw_led.blue;
    s_actuator_queue_data.data.rgbwled.white = data->rgbw_led.white;
    xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, 0);
    break;

  case UROS_SUB_IP_ADDR:
    s_lcd_queue_data.command = LCD_TASK_COMMAND_IP_ADDR;
    s_lcd_queue_data.data.ip_addr.ipv4 = data->ip_addr.value;
    xQueueSend(lcd_task_queue_hd, &s_lcd_queue_data, 0);

    if (s_last_ip != data->ip_addr.value) { // ip 주소 변경 시 uros 재시작 필요
      uros_set_domain_id(switch8_get_sum());
      s_last_ip = data->ip_addr.value;
    }
    break;

  case UROS_SUB_NONE:
  case UROS_SUB_MAX:
  default:
    break;
  }
}

static void s_uros_req_cb(
    const uros_srv_req_flag_t req_flag,
    const uros_srv_req_t *req,
    uros_srv_res_t *res) {
  switch (req_flag) {
  case UROS_SRV_ACTUATOR_ENABLE:
    s_actuator_queue_data.command = ACTUATOR_TASK_COMMAND_POWER;
    s_actuator_queue_data.data.power.power_target = true;
    s_actuator_queue_data.data.power.key_min_degree = req->actuator_enable.key_min_degree;
    s_actuator_queue_data.data.power.key_max_degree = req->actuator_enable.key_max_degree;
    s_actuator_queue_data.data.power.key_pulse_0_degree = req->actuator_enable.key_pulse_0_degree;
    s_actuator_queue_data.data.power.key_pulse_180_degree = req->actuator_enable.key_pulse_180_degree;
    s_actuator_queue_data.data.power.thruster_pulse_0_percentage = req->actuator_enable.thruster_pulse_0_percentage;
    s_actuator_queue_data.data.power.thruster_pulse_100_percentage = req->actuator_enable.thruster_pulse_100_percentage;
    xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, portMAX_DELAY);

    res->actuator_enable.status = true;
    break;

  case UROS_SRV_ACTUATOR_DISABLE:
    s_actuator_queue_data.command = ACTUATOR_TASK_COMMAND_POWER;
    s_actuator_queue_data.data.power.power_target = false;
    xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, portMAX_DELAY);

    res->actuator_disable.status = true;
    break;

  case UROS_SRV_TONE:
    s_actuator_queue_data.command = ACTUATOR_TASK_COMMAND_TONE;
    s_actuator_queue_data.data.tone.hz = req->tone.hz;
    s_actuator_queue_data.data.tone.duration_ms = req->tone.duration_ms;
    xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, 0);

    res->tone.status = xQueueSend(actuator_task_queue_hd, &s_actuator_queue_data, 0) == pdTRUE;
    break;

  case UROS_SRV_NONE:
  case UROS_SRV_MAX:
  default:
    break;
  }
}

static void s_uros_task(void *arg) {
  (void)arg;

  uint8_t domain_id;
  bool uros_last_status = false;
  uros_task_pub_queue_data_t uros_pub_queue_data;
  central_task_queue_data_t central_task_queue_data;
  central_task_queue_data.event_type = CENTRAL_TASK_EVENT_UROS_CONNECTION;

  uros_sub_set_callback(s_uros_sub_cb);
  uros_srv_set_callback(s_uros_req_cb);

  domain_id = switch8_get_sum();
  uros_set_domain_id(domain_id);

  for (;;) {
    // spin
    uros_spin();

    // ROS_DOMAIN_ID 체크 및 갱신
    if (domain_id != switch8_get_sum()) {
      domain_id = switch8_get_sum();
      uros_set_domain_id(switch8_get_sum());
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    // uros 연결 상태 공유
    if (uros_last_status != uros_is_connected()) {
      uros_last_status = uros_is_connected();

      central_task_queue_data.data.uros_connection.status = uros_last_status;
      xQueueSend(central_task_queue_hd, &central_task_queue_data, 0);
    }

    // publish request 수행
    if (xQueueReceive(uros_task_pub_queue_hd, &uros_pub_queue_data, 0) == pdTRUE && uros_is_connected()) {
      uros_pub_data_t buff;
      if (uros_pub_queue_data.topic == UROS_TASK_PUB_LOW_FREQ_SENSOR_DATA) {
        buff.emo_status.value = uros_pub_queue_data.data.low_freq_sensor_data.emo_status;
        uros_pub(UROS_PUB_EMO_STATUS, &buff);
        buff.battery_voltage.value = uros_pub_queue_data.data.low_freq_sensor_data.battery_voltage;
        uros_pub(UROS_PUB_BATTERY_VOLTAGE, &buff);
      } else if (uros_pub_queue_data.topic == UROS_TASK_PUB_POWER_OFF) {
        uros_pub(UROS_PUB_POWER_OFF_SIG, &buff);
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

bool uros_task_init(void) {
  uros_task_hd = xTaskCreateStatic(
      s_uros_task,
      "uros",
      UROS_TASK_SIZE,
      NULL,
      configEVENT_TASK_PRIORITIES,
      s_uros_task_buff,
      &s_uros_task_struct);

  return true;
}

bool uros_task_queue_init(void) {

  uros_task_pub_queue_hd = xQueueCreateStatic(
      UROS_TASK_PUB_QUEUE_LENGTH,
      UROS_TASK_PUB_QUEUE_ITEM_SIZE,
      s_uros_task_pub_queue_buff,
      &s_uros_task_pub_queue_struct);

  return true;
}