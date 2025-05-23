#include <stdio.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <timers.h>

#include "driver/lcd/lcd.h"
#include "driver/log/log.h"

#include "hal/time/time.h"

#include "lcd_task.h"

#define TAG "lcd_task"

typedef enum {
  SYSTEM_STATUS_0, // USB 연결 X
  SYSTEM_STATUS_1, // USB 연결 O
  SYSTEM_STATUS_2, // CDC 연결됨
  SYSTEM_STATUS_3, // UROS 연결됨
  SYSTEM_STATUS_4, // UROS 연결 해제됨
} system_status_t;

typedef struct {
  enum {
    LCD_TASK_COMMAND_NONE = 0x00,

    LCD_TASK_COMMAND_CTL_MODE,
    LCD_TASK_COMMAND_CONNECTION,
    LCD_TASK_COMMAND_ACT_POWER,
    LCD_TASK_COMMAND_IP_ADDR,
    LCD_TASK_COMMAND_KEY,
    LCD_TASK_COMMAND_THROTTLE,
    LCD_TASK_COMMAND_BAT_STATUS,
    LCD_TASK_COMMAND_POWER_OFF,

    LCD_TASK_COMMAND_FRAME,
    LCD_TASK_COMMAND_FORCE_REINIT,
    LCD_TASK_COMMAND_UPDATE_SYSTEM_STATUS,
    LCD_TASK_COMMAND_CHANGE_MAIN_INFO,
  } command;
  union {

    struct { // LCD_TASK_COMMAND_CTL_MODE
      lcd_task_ctl_mode_t mode;
    } ctl_mode;

    struct { // LCD_TASK_COMMAND_ACT_POWER
      bool status;
    } act_power;

    struct { // LCD_TASK_COMMAND_IP_ADDR
      uint32_t ipv4;
    } ip_addr;

    struct { // LCD_TASK_COMMAND_KEY
      uint8_t degree;
    } key;

    struct { // LCD_TASK_COMMAND_THROTTLE
      int8_t percentage;
    } throttle;

    struct { // LCD_TASK_COMMAND_BAT_STATUS
      uint8_t value;
    } bat_status;

    struct { // LCD_TASK_COMMAND_POWER_OFF
      uint8_t countdown;
      bool is_low_power;
    } power_off;

    struct { // LCD_TASK_COMMAND_FRAME
      uint8_t _;
    } frame;

    struct { // LCD_TASK_COMMAND_FORCE_REINIT
      uint8_t _;
    } force_reinit;

    struct { // LCD_TASK_COMMAND_UPDATE_SYSTEM_STATUS
      system_status_t status;
    } update_system_status;

    struct { // LCD_TASK_COMMAND_CHANGE_MAIN_INFO
      uint8_t _;
    } change_main_info;
  } data;
} lcd_task_queue_data_t;
#define LCD_TASK_QUEUE_LENGTH 20
#define LCD_TASK_QUEUE_ITEM_SIZE sizeof(lcd_task_queue_data_t)
static QueueHandle_t s_lcd_task_queue_hd = NULL;
static uint8_t s_lcd_task_queue_buff[LCD_TASK_QUEUE_LENGTH * LCD_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_lcd_task_queue_struct;

#define LCD_TASK_SIZE configMINIMAL_STACK_SIZE
static TaskHandle_t s_lcd_task_hd = NULL;
static StackType_t s_lcd_task_buff[LCD_TASK_SIZE];
static StaticTask_t s_lcd_task_struct;

static TimerHandle_t s_frame_generation_timer_hd = NULL;
static StaticTimer_t s_frame_generation_timer_buff;

static TimerHandle_t s_info_screen_toggle_timer_hd = NULL;
static StaticTimer_t s_info_screen_toggle_timer_buff;

static void s_frame_generation_timer_callback(TimerHandle_t timer_hd) {
  static TickType_t last_force_reinit_tick = 0;
  lcd_task_queue_data_t data;

  if (xTaskGetTickCount() - last_force_reinit_tick >= pdMS_TO_TICKS(10000)) {
    last_force_reinit_tick = xTaskGetTickCount();
    data.command = LCD_TASK_COMMAND_FORCE_REINIT;
  } else {
    data.command = LCD_TASK_COMMAND_FRAME;
  }

  xQueueSend(s_lcd_task_queue_hd, &data, 0);
}

static void s_info_screen_toggle_timer_callback(TimerHandle_t timer_hd) {
  lcd_task_queue_data_t data;

  data.command = LCD_TASK_COMMAND_CHANGE_MAIN_INFO;
  xQueueSend(s_lcd_task_queue_hd, &data, 0);
}

/**
 * @brief LCD 전원 종료 시퀀스 (관련없는 이벤트 모두 무시)
 *
 * @note TODO: 이쁜 방법은 아니지만... 잘 작동하니.. 일단 이대로 두고 나중에 수정합시다.
 *
 */
static void s_power_off(void) {
  lcd_task_queue_data_t lcd_queue_data = {0};
  char line_buff[16 + 1];

  xTimerStop(s_info_screen_toggle_timer_hd, 0); // turn off info_screen_toggle_timer

  for (;;) {
    if (xQueueReceive(s_lcd_task_queue_hd, &lcd_queue_data, portMAX_DELAY) == pdTRUE) {
      switch (lcd_queue_data.command) {
      case LCD_TASK_COMMAND_POWER_OFF:
        sprintf(line_buff, "Power off in %d", lcd_queue_data.data.power_off.countdown);
        for (uint8_t i = 0; i < 16; i++) { // fill blank
          if (line_buff[i] == 0) {
            for (uint8_t j = i; j < 16; j++) {
              line_buff[j] = ' ';
            }
            line_buff[16] = 0;
            break;
          }
        }

        lcd_set_cursor(0, 0);
        lcd_set_string(line_buff);

        if (lcd_queue_data.data.power_off.is_low_power) {
          lcd_set_cursor(1, 0);
          lcd_set_string("LOW POWER!!!    ");
        } else {
          lcd_set_cursor(1, 0);
          lcd_set_string("                ");
        }
        break;

      case LCD_TASK_COMMAND_FRAME:
        lcd_next_frame();
        break;

      case LCD_TASK_COMMAND_FORCE_REINIT:
        lcd_reinit_device();
        break;

      default:
        break;
      }
    }
  }
}

static void s_lcd_task(void *arg) {
  (void)arg;

  lcd_task_queue_data_t lcd_queue_data = {0};
  system_status_t system_status = SYSTEM_STATUS_0;
  bool status_ui_toggle = false; // true: ip 보여줌
  uint32_t ip_addr = 0;
  char line_buff[16 + 1];

  lcd_set_cursor(0, 0);
  lcd_set_string("Disconnected    ");
  lcd_set_cursor(1, 0);
  lcd_set_string("ACT OFF     B   ");

  xTimerStart(s_frame_generation_timer_hd, 0);
  xTimerStart(s_info_screen_toggle_timer_hd, 0);

  for (;;) {
    if (xQueueReceive(s_lcd_task_queue_hd, &lcd_queue_data, portMAX_DELAY) == pdTRUE) {
      switch (lcd_queue_data.command) {
      case LCD_TASK_COMMAND_CTL_MODE:
        lcd_set_cursor(0, 15);
        if (lcd_queue_data.data.ctl_mode.mode == LCD_TASK_CTL_MODE_RC) {
          lcd_set_char('M');
        } else if (lcd_queue_data.data.ctl_mode.mode == LCD_TASK_CTL_MODE_ROS) {
          lcd_set_char('A');
        } else {
          lcd_set_char('-');
        }
        break;

      case LCD_TASK_COMMAND_ACT_POWER:
        if (lcd_queue_data.data.act_power.status) {
          ;
        } else {
          lcd_set_cursor(1, 0);
          lcd_set_string("ACT OFF     ");
        }
        break;

      case LCD_TASK_COMMAND_IP_ADDR:
        ip_addr = lcd_queue_data.data.ip_addr.ipv4;

        status_ui_toggle = true;

        lcd_queue_data.command = LCD_TASK_COMMAND_CHANGE_MAIN_INFO;
        xQueueSendToFront(s_lcd_task_queue_hd, &lcd_queue_data, 0); // force update main info
        xTimerReset(s_info_screen_toggle_timer_hd, 0);
        break;

      case LCD_TASK_COMMAND_KEY:
        sprintf(line_buff + 5, " K%3d  ", lcd_queue_data.data.key.degree);
        lcd_set_cursor(1, 5);
        for (uint8_t i = 5; i < 12; i++) {
          lcd_set_char(line_buff[i]);
        }
        break;

      case LCD_TASK_COMMAND_THROTTLE:
        if (lcd_queue_data.data.throttle.percentage == 0) {
          sprintf(line_buff, "T OFF");
        } else if (lcd_queue_data.data.throttle.percentage > 0) {
          sprintf(line_buff, "T+%3d", lcd_queue_data.data.throttle.percentage);
        } else {
          lcd_queue_data.data.throttle.percentage *= -1;
          sprintf(line_buff, "T-%3d", lcd_queue_data.data.throttle.percentage);
        }
        lcd_set_cursor(1, 0);
        for (uint8_t i = 0; i < 5; i++) {
          lcd_set_char(line_buff[i]);
        }
        break;

      case LCD_TASK_COMMAND_BAT_STATUS:
        sprintf(line_buff, "B%3d",
                lcd_queue_data.data.bat_status.value);
        lcd_set_cursor(1, 12);
        lcd_set_string(line_buff);
        break;

      case LCD_TASK_COMMAND_POWER_OFF:
        xQueueSendToFront(s_lcd_task_queue_hd, &lcd_queue_data, 0);
        s_power_off();
        break;

      case LCD_TASK_COMMAND_FRAME:
        lcd_next_frame();
        break;

      case LCD_TASK_COMMAND_FORCE_REINIT:
        lcd_reinit_device();
        break;

      case LCD_TASK_COMMAND_UPDATE_SYSTEM_STATUS:
        if (lcd_queue_data.data.update_system_status.status == SYSTEM_STATUS_4) {
          if (system_status == SYSTEM_STATUS_3) {
            lcd_queue_data.data.update_system_status.status = SYSTEM_STATUS_2;
          }
        }

        status_ui_toggle = false;
        system_status = lcd_queue_data.data.update_system_status.status;

        lcd_queue_data.command = LCD_TASK_COMMAND_CHANGE_MAIN_INFO;
        xQueueSendToFront(s_lcd_task_queue_hd, &lcd_queue_data, 0); // force update main info
        xTimerReset(s_info_screen_toggle_timer_hd, 0);
        break;

      case LCD_TASK_COMMAND_CHANGE_MAIN_INFO:
        lcd_set_cursor(0, 0);
        if (system_status == SYSTEM_STATUS_0) { // USB Unplugged
          lcd_set_string("Disconnected   ");
        } else if (status_ui_toggle == true) {
          status_ui_toggle = !status_ui_toggle;
          if (ip_addr == 0) {
            sprintf(line_buff, "Waiting IP     ");
          } else {
            uint8_t octet1 = (ip_addr >> 24) & 0xFF;
            uint8_t octet2 = (ip_addr >> 16) & 0xFF;
            uint8_t octet3 = (ip_addr >> 8) & 0xFF;
            uint8_t octet4 = ip_addr & 0xFF;

            sprintf(line_buff, "%d.%d.%d.%d", octet1, octet2, octet3, octet4);
            for (uint8_t i = 0; i < 15; i++) { // fill blank
              if (line_buff[i] == 0) {
                for (uint8_t j = i; j < 15; j++) {
                  line_buff[j] = ' ';
                }
                line_buff[15] = 0;
                break;
              }
            }
          }
          lcd_set_string(line_buff);
        } else {
          status_ui_toggle = !status_ui_toggle;
          if (system_status == SYSTEM_STATUS_1) {
            lcd_set_string("USB Connected  ");
          } else if (system_status == SYSTEM_STATUS_2) {
            lcd_set_string("SBC Connected  ");
          } else if (system_status == SYSTEM_STATUS_3) {
            lcd_set_string("UROS Connected ");
          }
        }
        break;

      case LCD_TASK_COMMAND_NONE:
      default:
        break;
      }
    }
  }
}

bool lcd_task_init(void) {
  s_lcd_task_queue_hd = xQueueCreateStatic(
      LCD_TASK_QUEUE_LENGTH,
      LCD_TASK_QUEUE_ITEM_SIZE,
      s_lcd_task_queue_buff,
      &s_lcd_task_queue_struct);

  s_lcd_task_hd = xTaskCreateStatic(
      s_lcd_task,
      "lcd",
      LCD_TASK_SIZE,
      NULL,
      configEVENT_TASK_PRIORITIES,
      s_lcd_task_buff,
      &s_lcd_task_struct);

  s_frame_generation_timer_hd = xTimerCreateStatic(
      "frame_generation_timer",
      pdMS_TO_TICKS(200),
      pdTRUE,
      (void *)0,
      s_frame_generation_timer_callback,
      &s_frame_generation_timer_buff);

  s_info_screen_toggle_timer_hd = xTimerCreateStatic(
      "info_screen_toggle_timer",
      pdMS_TO_TICKS(2000),
      pdTRUE,
      (void *)0,
      s_info_screen_toggle_timer_callback,
      &s_info_screen_toggle_timer_buff);

  return true;
}

static bool s_send_queue(lcd_task_queue_data_t *queue_data) {
  bool ret = xQueueSend(s_lcd_task_queue_hd, queue_data, 0) == pdTRUE;
  if (ret == false) {
    log_warning(TAG, "Publish queue full!! message dropped!!");
  }

  return ret;
}

bool lcd_task_update_ctl_mode(lcd_task_ctl_mode_t mode) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_CTL_MODE;
  queue_data.data.ctl_mode.mode = mode;

  return s_send_queue(&queue_data);
}

bool lcd_task_update_actuator_power(bool status) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_ACT_POWER;
  queue_data.data.act_power.status = status;

  return s_send_queue(&queue_data);
}
bool lcd_task_update_ip_addr(uint32_t ipv4) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_IP_ADDR;
  queue_data.data.ip_addr.ipv4 = ipv4;

  return s_send_queue(&queue_data);
}

bool lcd_task_update_throttle(int8_t percentage) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_THROTTLE;
  queue_data.data.throttle.percentage = percentage;

  return s_send_queue(&queue_data);
}

bool lcd_task_update_key(uint8_t degree) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_KEY;
  queue_data.data.key.degree = degree;

  return s_send_queue(&queue_data);
}

bool lcd_task_update_battery(uint8_t percentage) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_BAT_STATUS;
  queue_data.data.bat_status.value = percentage;

  return s_send_queue(&queue_data);
}

bool lcd_task_update_power_off(uint8_t countdown, bool is_low_power) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_POWER_OFF;
  queue_data.data.power_off.countdown = countdown;
  queue_data.data.power_off.is_low_power = is_low_power;

  return s_send_queue(&queue_data);
}

void lcd_task_noti_usb_unplugged(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_UPDATE_SYSTEM_STATUS;
  queue_data.data.update_system_status.status = SYSTEM_STATUS_0;

  s_send_queue(&queue_data);
}

void lcd_task_noti_usb_plugged(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_UPDATE_SYSTEM_STATUS;
  queue_data.data.update_system_status.status = SYSTEM_STATUS_1;

  s_send_queue(&queue_data);
}

void lcd_task_noti_cdc_connected(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_UPDATE_SYSTEM_STATUS;
  queue_data.data.update_system_status.status = SYSTEM_STATUS_2;

  s_send_queue(&queue_data);
}

void lcd_task_noti_uros_connected(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_UPDATE_SYSTEM_STATUS;
  queue_data.data.update_system_status.status = SYSTEM_STATUS_3;

  s_send_queue(&queue_data);
}

void lcd_task_noti_uros_disconnected(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_UPDATE_SYSTEM_STATUS;
  queue_data.data.update_system_status.status = SYSTEM_STATUS_4;

  s_send_queue(&queue_data);
}