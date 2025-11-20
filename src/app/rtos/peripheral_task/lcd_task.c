#include <memory.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <timers.h>

#include "driver/lcd/lcd.h"
#include "driver/log/log.h"

#include "hal/time/time.h"

#include "sbc/sbc.h"

#include "lcd_task.h"

#define TAG "lcd_task"

typedef enum {
  LCD_MENU_MAIN = 0x00,
  LCD_MENU_NETWORK,

  LCD_MENU_MAX,
} lcd_menu_t;

typedef enum {
  USB_CONNECTION_0, // USB 연결 X
  USB_CONNECTION_1, // USB 연결 O
  USB_CONNECTION_2, // CDC 연결됨
  USB_CONNECTION_3, // UROS 연결됨
  USB_CONNECTION_4, // UROS 연결 해제됨
} usb_connection_t;

typedef struct {
  lcd_menu_t current_menu;

  bool info_swap; // 디스플레이 좁아서 n초 간격으로 정보 바꿈, 이때 사용함

  struct { // MENU_MAIN
    uint32_t ipv4;
    bool ipv4_changed; // swap 주요 정보 - swap 강제로 발생시키는 용도로 사용

    usb_connection_t usb_connection;
    bool usb_connection_changed; // swap 주요 정보

    lcd_task_ctl_mode_t control_mode;

    bool actuator_power;

    int8_t throttle;

    uint8_t key;

    uint8_t battery;
  } main;

  struct { // MENU_NETWORK
    lcd_task_network_info_t network_type;
    char ssid[17];
    int8_t rssi;
    uint16_t frequency;
    float ping_ms;
  } network;
} lcd_menu_data_t;

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
    LCD_TASK_COMMAND_NETWORK_INFO,
    LCD_TASK_COMMAND_PING,

    LCD_TASK_COMMAND_FRAME,
    LCD_TASK_COMMAND_FORCE_REINIT,
    LCD_TASK_COMMAND_UPDATE_USB_CONNECTION_STATUS,
    LCD_TASK_COMMAND_SWAP_INFO,

    LCD_TASK_COMMAND_BUTTON_CLICK,
    LCD_TASK_COMMAND_BUTTON_DOUBLE_CLICK,
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

    struct { // LCD_TASK_COMMAND_NETWORK_INFO
      lcd_task_network_info_t type;
      char ssid[17];
      int8_t rssi;
      uint16_t frequency;
    } network_info;

    struct { // LCD_TASK_COMMAND_PING
      float ping_ms;
    } ping;

    struct { // LCD_TASK_COMMAND_FRAME
      uint8_t _;
    } frame;

    struct { // LCD_TASK_COMMAND_FORCE_REINIT
      uint8_t _;
    } force_reinit;

    struct { // LCD_TASK_COMMAND_UPDATE_USB_CONNECTION_STATUS
      usb_connection_t status;
    } update_usb_connection_status;

    struct { // LCD_TASK_COMMAND_SWAP_INFO
      uint8_t _;
    } change_main_info;
  } data;
} lcd_task_queue_data_t;
#define LCD_TASK_QUEUE_LENGTH 20
#define LCD_TASK_QUEUE_ITEM_SIZE sizeof(lcd_task_queue_data_t)
static QueueHandle_t s_lcd_task_queue_hd = NULL;
static uint8_t s_lcd_task_queue_buff[LCD_TASK_QUEUE_LENGTH * LCD_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_lcd_task_queue_struct;

#define LCD_TASK_SIZE 1024
static TaskHandle_t s_lcd_task_hd = NULL;
static StackType_t s_lcd_task_buff[LCD_TASK_SIZE];
static StaticTask_t s_lcd_task_struct;

static TimerHandle_t s_frame_generation_timer_hd = NULL;
static StaticTimer_t s_frame_generation_timer_buff;

static TimerHandle_t s_info_swap_timer_hd = NULL;
static StaticTimer_t s_info_swap_timer_buff;

static TimerHandle_t s_network_menu_frequent_job_timer_hd = NULL;
static StaticTimer_t s_network_menu_frequent_job_timer_buff;

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

static void s_info_swap_timer_callback(TimerHandle_t timer_hd) {
  lcd_task_queue_data_t data;

  data.command = LCD_TASK_COMMAND_SWAP_INFO;
  xQueueSend(s_lcd_task_queue_hd, &data, 0);
}

static void s_network_menu_frequent_job_timer_callback(TimerHandle_t timer_hd) {
  mw_sbc_request_network_info();
}

static void s_network_info_response_callback(mw_sbc_network_status_t type, char *ssid, int8_t rssi, uint16_t frequency) {
  lcd_task_queue_data_t data = {0};
  data.command = LCD_TASK_COMMAND_NETWORK_INFO;
  data.data.network_info.type = type;
  strncpy(data.data.network_info.ssid, ssid, sizeof(char) * 16);
  data.data.network_info.rssi = rssi;
  data.data.network_info.frequency = frequency;

  xQueueSend(s_lcd_task_queue_hd, &data, 0);
}

static void s_ping_response_callback(float ping_ms) {
  lcd_task_queue_data_t data = {0};
  data.command = LCD_TASK_COMMAND_PING;
  data.data.ping.ping_ms = ping_ms;

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

  xTimerStop(s_info_swap_timer_hd, 0); // turn off info_swap_timer

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

static void s_do_double_click_work(lcd_menu_t current_menu) {
  switch (current_menu) {
  case LCD_MENU_NETWORK:
    mw_sbc_request_ping();
    break;

  case LCD_MENU_MAIN:
  case LCD_MENU_MAX:
  default:
    break;
  }
}

static void s_start_menu_task(lcd_menu_t current_menu) {
  switch (current_menu) {
  case LCD_MENU_NETWORK:
    mw_sbc_request_ping();
    mw_sbc_request_network_info();
    xTimerStart(s_network_menu_frequent_job_timer_hd, 0);
    break;

  case LCD_MENU_MAIN:
  case LCD_MENU_MAX:
  default:
    xTimerStop(s_network_menu_frequent_job_timer_hd, 0);
    break;
  }
}

static void s_lcd_update(lcd_menu_data_t *lcd_data) {
  switch (lcd_data->current_menu) {
  case LCD_MENU_MAIN:
    // 주요 정보 변경 처리 (swap 화면의 주요 정보 바뀌면 강제로 swap 발생+swap 트리거 타이머 초기화)
    if (lcd_data->main.usb_connection_changed == true) { // USB 연결상태 변화 처리가 우선순위 높음
      lcd_data->main.usb_connection_changed = false;

      lcd_data->info_swap = false;
      xTimerReset(s_info_swap_timer_hd, 0);
    } else if (lcd_data->main.ipv4_changed == true) {

      lcd_data->main.ipv4_changed = false;
      lcd_data->info_swap = true;
      xTimerReset(s_info_swap_timer_hd, 0);
    }

    // 연결 상태 표시 (swap==false)
    if (lcd_data->info_swap == false) {
      char line_buff[16 + 1] = {0};

      switch (lcd_data->main.usb_connection) {
      case USB_CONNECTION_0:
        sprintf(line_buff, "Disconnected   ");
        break;

      case USB_CONNECTION_1:
        sprintf(line_buff, "USB Connected  ");
        break;

      case USB_CONNECTION_2:
        sprintf(line_buff, "SBC Connected  ");
        break;

      case USB_CONNECTION_3:
        sprintf(line_buff, "UROS Connected ");
        break;

      case USB_CONNECTION_4:
        sprintf(line_buff, "SBC Connected  ");
        break;

      default:
        sprintf(line_buff, "               ");
        break;
      }

      lcd_set_cursor(0, 0);
      lcd_set_string(line_buff);
    }

    // IP 주소 표시 (swap==true && usb usb_connection >= USB_CONNECTION_2)
    if (lcd_data->info_swap == true && lcd_data->main.usb_connection >= USB_CONNECTION_2) {
      char line_buff[16 + 1] = {0};

      if (lcd_data->main.ipv4 == 0) {
        sprintf(line_buff, "Waiting IP     ");
      } else {
        uint8_t octet1 = (lcd_data->main.ipv4 >> 24) & 0xFF;
        uint8_t octet2 = (lcd_data->main.ipv4 >> 16) & 0xFF;
        uint8_t octet3 = (lcd_data->main.ipv4 >> 8) & 0xFF;
        uint8_t octet4 = lcd_data->main.ipv4 & 0xFF;
        sprintf(line_buff, "%d.%d.%d.%d", octet1, octet2, octet3, octet4);
      }
      for (uint8_t i = 0; i < 15; i++) { // fill blank
        if (line_buff[i] == 0) {
          for (uint8_t j = i; j < 15; j++) {
            line_buff[j] = ' ';
          }
          line_buff[15] = 0;
          break;
        }
      }

      lcd_set_cursor(0, 0);
      lcd_set_string(line_buff);
    }

    // control mode 처리
    {
      char c;

      switch (lcd_data->main.control_mode) {
      case LCD_TASK_CTL_MODE_NONE:
        c = '-';
        break;

      case LCD_TASK_CTL_MODE_RC:
        c = 'M';
        break;

      case LCD_TASK_CTL_MODE_ROS:
        c = 'A';
        break;

      default:
        break;
      }

      lcd_set_cursor(0, 15);
      lcd_set_char(c);
    }

    // actuator off 처리
    {
      if (lcd_data->main.actuator_power == false) {
        lcd_set_cursor(1, 0);
        lcd_set_string("ACT OFF     ");
      }
    }

    // 쓰러스터 처리 (actuator on)
    if (lcd_data->main.actuator_power == true) {
      char line_buff[16 + 1] = {0};

      if (lcd_data->main.throttle == 0) {
        sprintf(line_buff, "T OFF");
      } else if (lcd_data->main.throttle > 0) {
        sprintf(line_buff, "T+%3d", lcd_data->main.throttle);
      } else {
        lcd_data->main.throttle *= -1;
        sprintf(line_buff, "T-%3d", lcd_data->main.throttle);
      }

      lcd_set_cursor(1, 0);
      for (uint8_t i = 0; i < 5; i++) {
        lcd_set_char(line_buff[i]);
      }
    }

    // 키 처리 (actuator on)
    if (lcd_data->main.actuator_power == true) {
      char line_buff[16 + 1] = {0};

      sprintf(line_buff + 5, " K%3d  ", lcd_data->main.key);

      lcd_set_cursor(1, 5);
      for (uint8_t i = 5; i < 12; i++) {
        lcd_set_char(line_buff[i]);
      }
    }

    // 배터리 처리
    {
      char line_buff[16 + 1] = {0};

      sprintf(line_buff, "B%3d", lcd_data->main.battery);

      lcd_set_cursor(1, 12);
      lcd_set_string(line_buff);
    }
    break;

  case LCD_MENU_NETWORK:
    if (lcd_data->main.usb_connection <= USB_CONNECTION_1) {
      lcd_set_cursor(0, 0);
      lcd_set_string("Network info ---");
      lcd_set_cursor(1, 0);
      lcd_set_string("SBC Disconnected");
      break;
    }

    // SSID, IP (swap)
    {
      char line_buff[16 + 1] = {0};

      if (lcd_data->info_swap == false && lcd_data->network.network_type == LCD_TASK_NETWORK_INFO_WLAN) { // SSID
        memcpy(line_buff, lcd_data->network.ssid, 17);
      } else { // IP
        if (lcd_data->main.ipv4 == 0) {
          sprintf(line_buff, "Waiting IP");
        } else {
          uint8_t octet1 = (lcd_data->main.ipv4 >> 24) & 0xFF;
          uint8_t octet2 = (lcd_data->main.ipv4 >> 16) & 0xFF;
          uint8_t octet3 = (lcd_data->main.ipv4 >> 8) & 0xFF;
          uint8_t octet4 = lcd_data->main.ipv4 & 0xFF;
          sprintf(line_buff, "%d.%d.%d.%d", octet1, octet2, octet3, octet4);
        }
      }

      for (uint8_t i = 0; i < 17; i++) { // fill blank
        if (line_buff[i] == 0) {
          for (uint8_t j = i; j < 17; j++) {
            line_buff[j] = ' ';
          }
          line_buff[17] = 0;
          break;
        }
      }

      lcd_set_cursor(0, 0);
      lcd_set_string(line_buff);
    }

    // rssi, frequency (swap)
    {
      char line_buff[16 + 1] = {0};

      if (lcd_data->network.network_type == LCD_TASK_NETWORK_INFO_NONE) {
        sprintf(line_buff, "NONE    %6.1fms", lcd_data->network.ping_ms);
      } else if (lcd_data->network.network_type == LCD_TASK_NETWORK_INFO_LAN) {
        sprintf(line_buff, "LAN     %6.1fms", lcd_data->network.ping_ms);
      } else {
        if (lcd_data->info_swap == false) { // rssi
          sprintf(line_buff, "%4ddBm %6.1fms", (int)lcd_data->network.rssi, lcd_data->network.ping_ms);
        } else { // channel
          sprintf(line_buff, "%4dmHz %6.1fms", (int)lcd_data->network.frequency, lcd_data->network.ping_ms);
        }
      }

      lcd_set_cursor(1, 0);
      lcd_set_string(line_buff);
    }

    break;
  case LCD_MENU_MAX:
  default:
    break;
  }
}

static void s_lcd_task(void *arg) {
  (void)arg;

  mw_sbc_set_network_info_response_callback(s_network_info_response_callback);
  mw_sbc_set_ping_response_callback(s_ping_response_callback);

  lcd_menu_data_t lcd_data = {0};
  lcd_task_queue_data_t lcd_queue_data = {0};

  // default
  lcd_data.current_menu = LCD_MENU_MAIN;
  strcpy(lcd_data.network.ssid, "**LOADING**"); // TODO: 모드 종료 콜백같은걸 만들어서 거기에도 집어넣을것

  xTimerStart(s_frame_generation_timer_hd, 0);
  xTimerStart(s_info_swap_timer_hd, 0);

  for (;;) {
    if (xQueueReceive(s_lcd_task_queue_hd, &lcd_queue_data, portMAX_DELAY) == pdTRUE) {
      switch (lcd_queue_data.command) {
      case LCD_TASK_COMMAND_CTL_MODE:
        lcd_data.main.control_mode = lcd_queue_data.data.ctl_mode.mode;
        break;

      case LCD_TASK_COMMAND_CONNECTION:
        lcd_data.main.usb_connection = lcd_queue_data.data.update_usb_connection_status.status;
        lcd_data.main.usb_connection_changed = true;
        break;

      case LCD_TASK_COMMAND_ACT_POWER:
        lcd_data.main.actuator_power = lcd_queue_data.data.act_power.status;
        break;

      case LCD_TASK_COMMAND_IP_ADDR:
        lcd_data.main.ipv4 = lcd_queue_data.data.ip_addr.ipv4;
        lcd_data.main.ipv4_changed = true;
        break;

      case LCD_TASK_COMMAND_KEY:
        lcd_data.main.key = lcd_queue_data.data.key.degree;
        break;

      case LCD_TASK_COMMAND_THROTTLE:
        lcd_data.main.throttle = lcd_queue_data.data.throttle.percentage;
        break;

      case LCD_TASK_COMMAND_BAT_STATUS:
        lcd_data.main.battery = lcd_queue_data.data.bat_status.value;
        break;

      case LCD_TASK_COMMAND_POWER_OFF:
        xQueueSendToFront(s_lcd_task_queue_hd, &lcd_queue_data, 0);
        s_power_off();
        break;

      case LCD_TASK_COMMAND_NETWORK_INFO:
        lcd_data.network.network_type = lcd_queue_data.data.network_info.type;
        memcpy(lcd_data.network.ssid, lcd_queue_data.data.network_info.ssid, sizeof(char) * 17);
        lcd_data.network.rssi = lcd_queue_data.data.network_info.rssi;
        lcd_data.network.frequency = lcd_queue_data.data.network_info.frequency;
        break;

      case LCD_TASK_COMMAND_PING:
        lcd_data.network.ping_ms = lcd_queue_data.data.ping.ping_ms;
        break;

      case LCD_TASK_COMMAND_FRAME:
        s_lcd_update(&lcd_data);
        lcd_next_frame();
        break;

      case LCD_TASK_COMMAND_FORCE_REINIT:
        s_lcd_update(&lcd_data);
        lcd_reinit_device();
        break;

      case LCD_TASK_COMMAND_UPDATE_USB_CONNECTION_STATUS:
        // if (lcd_queue_data.data.update_usb_connection_status.status == USB_CONNECTION_4 && lcd_data.main.usb_connection == USB_CONNECTION_3) { // TODO: 없어도 괜찮을듯. 테스트해볼것.
        //   lcd_queue_data.data.update_usb_connection_status.status = USB_CONNECTION_2;
        // }
        lcd_data.main.usb_connection = lcd_queue_data.data.update_usb_connection_status.status;
        lcd_data.main.usb_connection_changed = true;
        break;

      case LCD_TASK_COMMAND_SWAP_INFO:
        lcd_data.info_swap = !lcd_data.info_swap;
        break;

      case LCD_TASK_COMMAND_BUTTON_CLICK:
        lcd_data.info_swap = false;
        if (++lcd_data.current_menu >= LCD_MENU_MAX) {
          lcd_data.current_menu = LCD_MENU_MAIN;
        }

        s_start_menu_task(lcd_data.current_menu);
        xTimerReset(s_info_swap_timer_hd, 0);
        lcd_clear();
        break;

      case LCD_TASK_COMMAND_BUTTON_DOUBLE_CLICK:
        s_do_double_click_work(lcd_data.current_menu);
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

  s_info_swap_timer_hd = xTimerCreateStatic(
      "info_swap_timer",
      pdMS_TO_TICKS(2000),
      pdTRUE,
      (void *)0,
      s_info_swap_timer_callback,
      &s_info_swap_timer_buff);

  s_network_menu_frequent_job_timer_hd = xTimerCreateStatic(
      "network_menu_frequent_job_timer",
      pdMS_TO_TICKS(500),
      pdTRUE,
      (void *)0,
      s_network_menu_frequent_job_timer_callback,
      &s_network_menu_frequent_job_timer_buff);

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
  queue_data.command = LCD_TASK_COMMAND_UPDATE_USB_CONNECTION_STATUS;
  queue_data.data.update_usb_connection_status.status = USB_CONNECTION_0;

  s_send_queue(&queue_data);
}

void lcd_task_noti_usb_plugged(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_UPDATE_USB_CONNECTION_STATUS;
  queue_data.data.update_usb_connection_status.status = USB_CONNECTION_1;

  s_send_queue(&queue_data);
}

void lcd_task_noti_cdc_connected(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_UPDATE_USB_CONNECTION_STATUS;
  queue_data.data.update_usb_connection_status.status = USB_CONNECTION_2;

  s_send_queue(&queue_data);
}

void lcd_task_noti_uros_connected(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_UPDATE_USB_CONNECTION_STATUS;
  queue_data.data.update_usb_connection_status.status = USB_CONNECTION_3;

  s_send_queue(&queue_data);
}

void lcd_task_noti_uros_disconnected(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_UPDATE_USB_CONNECTION_STATUS;
  queue_data.data.update_usb_connection_status.status = USB_CONNECTION_4;

  s_send_queue(&queue_data);
}

void lcd_task_noti_button_clicked(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_BUTTON_CLICK;

  s_send_queue(&queue_data);
}

void lcd_task_noti_button_dclicked(void) {
  lcd_task_queue_data_t queue_data;
  queue_data.command = LCD_TASK_COMMAND_BUTTON_DOUBLE_CLICK;

  s_send_queue(&queue_data);
}
