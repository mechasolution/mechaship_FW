#include <math.h>
#include <stdatomic.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "rtos/central_task/central_task.h"
#include "rtos/peripheral_task/actuator_task.h"
#include "rtos/peripheral_task/lcd_task.h"
#include "rtos/peripheral_task/sled_task.h"

#include "driver/battery/battery.h"
#include "driver/emo/emo.h"
#include "driver/log/log.h"
#include "driver/switch8/switch8.h"

#include "hal/time/time.h"

#include "sbc/sbc.h"
#include "uros/uros.h"

#include "uros_task.h"

#define TAG "uros_task"

#define UROS_TASK_SIZE 4096
static TaskHandle_t s_uros_task_hd = NULL;
static StackType_t s_uros_task_buff[UROS_TASK_SIZE];
static StaticTask_t s_uros_task_struct;
static SemaphoreHandle_t s_task_stop_ack;
static StaticSemaphore_t s_task_stop_ack_storage;
static atomic_bool s_task_running = false;
static atomic_bool s_task_stopped = true;
static bool s_task_stop_result = true;

static atomic_bool s_is_act_enabled = false;
static void s_uros_sub_cb(
    const uros_sub_data_flag_t data_flag,
    const uros_sub_data_t *data) {

  switch (data_flag) {
  case UROS_SUB_KEY:
    if (s_is_act_enabled == true) {
      actuator_task_set_key(data->key_degree.degree);
      lcd_task_update_key((uint8_t)data->key_degree.degree);
    }
    break;

  case UROS_SUB_THROTTLE:
    if (s_is_act_enabled == true) {
      actuator_task_set_throttle(data->throttle_percentage.percentage);
      lcd_task_update_throttle((int8_t)data->throttle_percentage.percentage);
    }
    break;

  case UROS_SUB_TONE:
    actuator_task_set_tone(
        data->tone.hz,
        data->tone.duration_ms);
    break;

  case UROS_SUB_RGBW_LED:
    actuator_task_set_rgbwled(data->rgbw_led.red,
                              data->rgbw_led.green,
                              data->rgbw_led.blue,
                              data->rgbw_led.white);
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
    if (!atomic_load(&s_task_running)) {
      res->actuator_enable.status = false;
      break;
    }
    res->actuator_enable.status = actuator_task_set_power(true,
                                                          req->actuator_enable.key_min_degree,
                                                          req->actuator_enable.key_max_degree,
                                                          req->actuator_enable.key_pulse_0_degree,
                                                          req->actuator_enable.key_pulse_180_degree,
                                                          req->actuator_enable.thruster_pulse_0_percentage,
                                                          req->actuator_enable.thruster_pulse_100_percentage);
    s_is_act_enabled = res->actuator_enable.status;
    break;

  case UROS_SRV_ACTUATOR_DISABLE:
    s_is_act_enabled = false;
    res->actuator_disable.status = actuator_task_set_power(false, 0, 0, 0, 0, 0, 0);
    break;

  case UROS_SRV_TONE:
    res->tone.status = actuator_task_set_tone(req->tone.hz,
                                              req->tone.duration_ms);
    break;

  case UROS_SRV_NONE:
  case UROS_SRV_MAX:
  default:
    break;
  }
}

static void s_task_start_melody(void) {
  for (uint8_t i = 0; i < 3; i++) {
    actuator_task_set_tone(739, 150);
    actuator_task_set_tone(0, 150);
  }
  actuator_task_set_tone(0, 500);
}

static void s_uros_task(void *arg) {
  (void)arg;

  for (;;) {
    while (!atomic_load(&s_task_running)) {
      if (!atomic_load(&s_task_stopped)) {
        s_task_stop_result = true; // Stopped before the session started.
        atomic_store(&s_task_stopped, true);
        xSemaphoreGive(s_task_stop_ack);
      }
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    ulTaskNotifyTake(pdTRUE, 0); // Discard IP notifications from the previous run.
    s_is_act_enabled = false;

    bool uros_connection = false;
    uros_pub_data_t buff;
    uint8_t domain_id = switch8_get_sum();
    TickType_t last_sbc_domain_id_send_tick = 0;

    sled_task_set_pattern(SLED_TASK_PATTERN_REFLASH);
    s_task_start_melody();

    static bool is_callback_set = false;
    if (is_callback_set == false) {
      is_callback_set = true;
      uros_sub_set_callback(s_uros_sub_cb);
      uros_srv_set_callback(s_uros_req_cb);
    }

    uros_set_domain_id(domain_id);
    bool worker_started = uros_init();
    if (!worker_started) log_error(TAG, "micro-ROS worker did not start");

    while (atomic_load(&s_task_running)) {
      vTaskDelay(pdMS_TO_TICKS(100));
      if (!atomic_load(&s_task_running)) break;
      if (!worker_started) {
        worker_started = uros_init();
        continue;
      }

      bool curr_connection = uros_is_connected();

      // connection change sled status update
      if (uros_connection != curr_connection) {
        uros_connection = curr_connection;
        if (curr_connection == true) {
          lcd_task_noti_uros_connected();
          sled_task_set_pattern(SLED_TASK_PATTERN_SOLID);
        } else {
          s_is_act_enabled = false;
          lcd_task_noti_uros_disconnected();
          actuator_task_set_power(false, 0, 0, 0, 0, 0, 0);
          sled_task_set_pattern(SLED_TASK_PATTERN_REFLASH);
        }
      }

      // check domain id and update when needed
      uint8_t domain_id_temp = switch8_get_sum();
      if (domain_id != domain_id_temp) {
        domain_id = domain_id_temp;

        uros_set_domain_id(domain_id);
        mw_sbc_report_domain_id(domain_id);
        last_sbc_domain_id_send_tick = xTaskGetTickCount();

        if (curr_connection == true) {
          s_is_act_enabled = false;
          actuator_task_set_power(false, 0, 0, 0, 0, 0, 0);
          bool stopped = uros_deinit();
          vTaskDelay(pdMS_TO_TICKS(10)); // good to have
          worker_started = atomic_load(&s_task_running) && stopped && uros_init();
          if (!worker_started) log_error(TAG, "micro-ROS domain restart failed");

          continue; // 이 시점에 분명히 연결 끊어져있음.
        }
      }

      // report domain id to sbc
      // TODO: SBC와 연결되는 시점에 바로 전송 (task alert 추가)
      if (xTaskGetTickCount() - last_sbc_domain_id_send_tick >= pdMS_TO_TICKS(5000)) {
        last_sbc_domain_id_send_tick = xTaskGetTickCount();
        mw_sbc_report_domain_id(domain_id);
      }

      // if uros disconnected, continue
      if (curr_connection == false) {
        s_is_act_enabled = false;
        continue;
      }

      // if ip changed, reinit uros due to rmw issue (related discussion: https://github.com/ros2/rmw/pull/344#issuecomment-1398861272)
      int temp = ulTaskNotifyTake(pdTRUE, 0);
      if (temp != 0) {
        s_is_act_enabled = false;
        lcd_task_noti_uros_disconnected();
        actuator_task_set_power(false, 0, 0, 0, 0, 0, 0);
        actuator_task_set_rgbwled(0, 0, 0, 0);

        bool stopped = uros_deinit();
        vTaskDelay(pdMS_TO_TICKS(500));
        worker_started = atomic_load(&s_task_running) && stopped && uros_init();
        if (!worker_started) log_error(TAG, "micro-ROS IP restart failed");

        uros_connection = false;

        continue;
      }

      buff.battery_voltage.value = battery_get_voltage();
      uros_pub(UROS_PUB_BATTERY_VOLTAGE, &buff);

      buff.emo_status.value = emo_get_status();
      uros_pub(UROS_PUB_EMO_STATUS, &buff);
    }
    s_is_act_enabled = false;
    s_task_stop_result = uros_deinit();
    atomic_store(&s_task_stopped, true);
    xSemaphoreGive(s_task_stop_ack);
  }
}

bool uros_task_init(void) {
  if (atomic_load(&s_task_running) || !atomic_load(&s_task_stopped)) return false;
  if (s_task_stop_ack == NULL) s_task_stop_ack = xSemaphoreCreateBinaryStatic(&s_task_stop_ack_storage);
  if (s_task_stop_ack == NULL) return false;
  if (s_uros_task_hd == NULL) {
    s_uros_task_hd = xTaskCreateStatic(
        s_uros_task, "uros", UROS_TASK_SIZE, NULL, configIDLE_TASK_PRIORITIES,
        s_uros_task_buff, &s_uros_task_struct);
  }
  if (s_uros_task_hd == NULL) return false;
  atomic_store(&s_task_stopped, false);
  atomic_store(&s_task_running, true);
  return true;
}

bool uros_task_deinit(void) {
  if (!atomic_exchange(&s_task_running, false)) return atomic_load(&s_task_stopped) && s_task_stop_result;
  s_is_act_enabled = false;
  lcd_task_noti_uros_disconnected();
  actuator_task_set_power(false, 0, 0, 0, 0, 0, 0);
  xSemaphoreTake(s_task_stop_ack, 0);
  if (xSemaphoreTake(s_task_stop_ack, pdMS_TO_TICKS(10000)) != pdTRUE) {
    log_error(TAG, "micro-ROS task stop timed out");
    return false;
  }
  s_is_act_enabled = false;
  bool power_off = actuator_task_set_power(false, 0, 0, 0, 0, 0, 0);
  actuator_task_set_rgbwled(0, 0, 0, 0);
  return s_task_stop_result && power_off;
}

void uros_task_noti_ip_changed(void) {
  if (s_uros_task_hd == NULL || !atomic_load(&s_task_running)) {
    return;
  }

  xTaskNotifyGive(s_uros_task_hd);
}
