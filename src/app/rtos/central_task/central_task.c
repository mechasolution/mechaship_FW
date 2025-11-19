#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "rtos/peripheral_task/actuator_task.h"
#include "rtos/peripheral_task/lcd_task.h"
#include "rtos/uros_task/uros_task.h"

#include "driver/battery/battery.h"
#include "driver/led/led.h"
#include "driver/log/log.h"
#include "driver/power/power.h"
#include "driver/rc4/rc4.h"

#include "central_task.h"
#include "rtos/peripheral_task/sled_task.h"
#include "rtos/rc_task/rc_task.h"
#include "rtos/uros_task/uros_task.h"

#include "sbc/sbc.h"

#define TAG "central"

#define SWITCH_DOUBLE_CLICK_PERIOS_MS 100

typedef struct {
  enum {
    CENTRAL_TASK_COMMAND_NONE = 0x00,

    CENTRAL_TASK_COMMAND_IP,
    CENTRAL_TASK_COMMAND_SBC_CONNECTION,
    CENTRAL_TASK_COMMAND_POWER_OFF_REQUEST,
  } command;
  union {
    struct { // CENTRAL_TASK_COMMAND_IP
      uint32_t ipv4;
    } ip;

    struct { // CENTRAL_TASK_COMMAND_SBC_CONNECTION
      bool status;
    } sbc_connection;

    struct { // CENTRAL_TASK_COMMAND_POWER_OFF_REQUEST
      bool _;
    } power_off_request;
  } data;
} central_task_queue_data_t;
#define CENTRAL_TASK_QUEUE_LENGTH 4
#define CENTRAL_TASK_QUEUE_ITEM_SIZE sizeof(central_task_queue_data_t)
static QueueHandle_t s_central_task_queue_hd;
static uint8_t s_central_task_queue_buff[CENTRAL_TASK_QUEUE_LENGTH * CENTRAL_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_central_task_queue_struct;

#define CENTRAL_TASK_SIZE (1024)
static TaskHandle_t s_central_task_hd = NULL;
static StackType_t s_central_task_buff[CENTRAL_TASK_SIZE];
static StaticTask_t s_central_task_struct;

static void s_ip_changed_melody(void) {
  for (uint8_t i = 0; i < 3; i++) {
    actuator_task_set_tone(739, 100);
    actuator_task_set_tone(0, 20);
  }
  actuator_task_set_tone(0, 500);
}

static void s_sbc_ip_change_cb(uint32_t ipv4) {
  central_task_queue_data_t data;
  data.command = CENTRAL_TASK_COMMAND_IP;
  data.data.ip.ipv4 = ipv4;
  bool ret = xQueueSend(s_central_task_queue_hd, &data, 0);

  if (ret == false) {
    log_warning(TAG, "Publish queue full!! message dropped!!");
  }
}

static void s_usb_disconnected_melody(void) {
  actuator_task_set_tone(739, 80);
  actuator_task_set_tone(0, 150);
  actuator_task_set_tone(523, 80);
  actuator_task_set_tone(0, 500);
}

static void s_usb_connected_melody(void) {
  actuator_task_set_tone(523, 80);
  actuator_task_set_tone(0, 150);
  actuator_task_set_tone(739, 80);
  actuator_task_set_tone(0, 500);
}

static void s_sbc_connection_change_cb(mw_sbc_connection_status_t status) {
  static bool connection_last = false;
  switch (status) {
  case MW_SBC_CONNECTION_NONE:
    lcd_task_noti_usb_unplugged();
    xTaskNotifyGive(s_central_task_hd);

    if (connection_last == true) {
      s_usb_disconnected_melody();
    }
    connection_last = false;
    break;

  case MW_SBC_CONNECTION_USB:
    lcd_task_noti_usb_plugged();

    if (connection_last == true) {
      s_usb_disconnected_melody();
    }
    connection_last = false;
    break;

  case MW_SBC_CONNECTION_CDC:
    lcd_task_noti_cdc_connected();

    if (connection_last == false) {
      s_usb_connected_melody();
    }
    connection_last = true;
    break;

  default:
    break;
  }
}

static void s_power_off(bool is_low_power) {
  log_warning(TAG, "Power off sequence start");

  ulTaskNotifyTake(pdTRUE, 0);

  led_set_rc_mode(false);
  led_set_ros_mode(false);

  sled_task_set_pattern(SLED_TASK_PATTERN_BLINK_FAST);
  actuator_task_set_power(false, 0, 0, 0, 0, 0, 0);
  actuator_task_set_tone(2000, 3000);

  mw_sbc_report_power_off();

  for (int i = 60; i >= 0; i--) {
    log_warning(TAG, "Power down in %d seconds", i);
    lcd_task_update_power_off(i, is_low_power);

    int temp = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));
    if (temp != 0) {
      i = 3;
    }
  }

  power_set_sbc(false);
  power_set_main(false);
}

typedef enum {
  MODE_NO_RC_MODULE = RC4_SLIDESWITCH_ERR,

  MODE_NONE = RC4_SLIDESWITCH_MIDDLE,
  MODE_RC = RC4_SLIDESWITCH_BACKWARD,
  MODE_ROS = RC4_SLIDESWITCH_FORWARD,
} mode_t;

static void s_mode_none_melody(void) {
  actuator_task_set_tone(739, 150);
  actuator_task_set_tone(622, 150);
  actuator_task_set_tone(523, 150);
  actuator_task_set_tone(0, 500);
}

static void s_update_mode(mode_t curr_mode) {
  switch (curr_mode) {
  case MODE_RC:
    log_debug(TAG, "RC MODE");
    led_set_rc_mode(true);
    led_set_ros_mode(false);
    lcd_task_update_ctl_mode(LCD_TASK_CTL_MODE_RC);

    uros_task_deinit();
    rc_task_init();
    break;

  case MODE_ROS:
    log_debug(TAG, "ROS MODE");

    led_set_rc_mode(false);
    led_set_ros_mode(true);
    lcd_task_update_ctl_mode(LCD_TASK_CTL_MODE_ROS);

    rc_task_deinit();
    uros_task_init();
    break;

  case MODE_NONE:
  default:
    log_debug(TAG, "IDLE MODE");
    sled_task_set_pattern(SLED_TASK_PATTERN_BLINK_SLOW);
    s_mode_none_melody();

    led_set_rc_mode(false);
    led_set_ros_mode(false);
    lcd_task_update_ctl_mode(LCD_TASK_CTL_MODE_NONE);

    rc_task_deinit();
    uros_task_deinit();
    break;
  }
}

static void s_process_event(central_task_queue_data_t *queue_data, mode_t current_mode) {
  switch (queue_data->command) {
  case CENTRAL_TASK_COMMAND_IP: // ip changed
    if (queue_data->data.ip.ipv4 != 0 && current_mode == MODE_ROS) {
      s_ip_changed_melody();
      uros_task_noti_ip_changed();
    }

    lcd_task_update_ip_addr(queue_data->data.ip.ipv4);
    break;

  case CENTRAL_TASK_COMMAND_POWER_OFF_REQUEST:
    s_power_off(false);
    break;

  case CENTRAL_TASK_COMMAND_NONE:
  default:
    break;
  }
}

static mode_t s_process_mode_change(void) {
  static mode_t current_mode = MODE_NONE;
  static mode_t mode_temp = MODE_NONE;
  static TickType_t last_mode_tick = 0;

  mode_t mode_target = rc4_get_slideswitch();
  if (mode_target == MODE_NO_RC_MODULE) {
    if (current_mode != MODE_ROS) {
      s_update_mode(MODE_ROS);
      current_mode = MODE_ROS;
    }
  } else {
    if (mode_target != current_mode) {
      if (mode_target != mode_temp) {
        mode_temp = mode_target;
        last_mode_tick = xTaskGetTickCount();
      } else if (xTaskGetTickCount() - last_mode_tick >= pdMS_TO_TICKS(300)) {
        s_update_mode(mode_temp);
        current_mode = mode_temp;
      }
    }
  }

  return current_mode;
}

static void s_process_switch_short_clicks(uint8_t cnt) {
  ;
}

static void s_process_switch(void) {
  static TickType_t press_start_tick = 0; // 누르기 시작한 시간
  static TickType_t press_end_tick = 0;   // 스위치 접점 떨어진 시간
  static uint8_t click_count = 0;
  static bool last_switch = false;

  bool current_switch = power_get_button();

  if (last_switch == current_switch) { // 상태 변하지 않은 경우
    // 스위치 누른 상태에서 5초 이상 경과 -> 길게 누르기 처리
    if (current_switch == true && xTaskGetTickCount() - press_start_tick >= pdMS_TO_TICKS(5000)) {
      s_power_off(false);
    }

    // 스위치 누르지 않은 상태에서 일정 시간 경과 -> 더블/싱글클릭 처리
    if (current_switch == false && click_count != 0 && xTaskGetTickCount() - press_end_tick > pdMS_TO_TICKS(SWITCH_DOUBLE_CLICK_PERIOS_MS)) {
      click_count = (click_count + 1) / 2;
      s_process_switch_short_clicks(click_count);
      click_count = 0;
    }
  } else { // 상태 변한경우 -> 싱글, 더블클릭 카운트
    last_switch = current_switch;
    if (current_switch == true) { // 스위치 누른 상황
      press_start_tick = xTaskGetTickCount();
      if (xTaskGetTickCount() - press_end_tick <= pdMS_TO_TICKS(10)) { // debounce
        ;
      } else {
        click_count++;
      }
    } else { // 스위치 땐 상황
      press_end_tick = xTaskGetTickCount();
      if (xTaskGetTickCount() - press_start_tick <= pdMS_TO_TICKS(10)) { // debounce
        ;
      } else if (xTaskGetTickCount() - press_start_tick <= pdMS_TO_TICKS(SWITCH_DOUBLE_CLICK_PERIOS_MS)) {
        click_count++;
      }
    }
  }
}

static void s_process_low_freq_task(void) {
  // watch battery
  float voltage = battery_get_voltage();
  float percentage = battery_get_percentage();

  if (percentage <= 0) {
    s_power_off(true);
  }

  // update lcd
  lcd_task_update_battery((uint8_t)percentage);

  // update sbc
  mw_sbc_report_battery_info(voltage, percentage);
}

static void s_power_off_request_event_cb(void) {
  central_task_queue_data_t data;
  data.command = CENTRAL_TASK_COMMAND_POWER_OFF_REQUEST;
  bool ret = xQueueSend(s_central_task_queue_hd, &data, 0);

  if (ret == false) {
    log_warning(TAG, "Publish queue full!! message dropped!!");
  }
}

static void s_central_task(void *arg) {
  central_task_queue_data_t queue_data = {0};
  TickType_t last_low_freq_work_tick = 0;

  sled_task_set_pattern(SLED_TASK_PATTERN_BLINK_SLOW);
  lcd_task_update_ctl_mode(LCD_TASK_CTL_MODE_NONE);
  mw_sbc_set_ipv4_change_callback(s_sbc_ip_change_cb);
  mw_sbc_set_connection_change_callback(s_sbc_connection_change_cb);
  mw_sbc_set_power_off_request_callback(s_power_off_request_event_cb);

  vTaskDelay(pdMS_TO_TICKS(100));

  for (;;) {
    // update mode
    mode_t current_mode = s_process_mode_change();

    // handle ip change event
    if (xQueueReceive(s_central_task_queue_hd, &queue_data, pdMS_TO_TICKS(50)) == pdTRUE) {
      s_process_event(&queue_data, current_mode);
    }

    // watch switch
    s_process_switch();

    // low freq work
    if (xTaskGetTickCount() - last_low_freq_work_tick >= pdMS_TO_TICKS(1000)) {
      last_low_freq_work_tick = xTaskGetTickCount();
      s_process_low_freq_task();
    }
  }
}

bool central_task_init(void) {
  s_central_task_queue_hd = xQueueCreateStatic(
      CENTRAL_TASK_QUEUE_LENGTH,
      CENTRAL_TASK_QUEUE_ITEM_SIZE,
      s_central_task_queue_buff,
      &s_central_task_queue_struct);

  s_central_task_hd = xTaskCreateStatic(
      s_central_task,
      "central",
      CENTRAL_TASK_SIZE,
      NULL,
      configIDLE_TASK_PRIORITIES,
      s_central_task_buff,
      &s_central_task_struct);

  return true;
}
