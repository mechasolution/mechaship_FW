#include <stdio.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <tusb.h>

#include "driver/log/log.h"

#include "sbc.h"

#define TAG "mw/sbc"

typedef struct {
  enum {
    SBC_TASK_COMMAND_NONE = 0x00,

    SBC_TASK_COMMAND_BATTERY_INFO,
    SBC_TASK_COMMAND_POWER_OFF,
  } command;

  union {
    struct { // SBC_TASK_COMMAND_BATTERY_INFO
      float voltage;
      float percentage;
    } battery_info;

    struct {  // SBC_TASK_COMMAND_POWER_OFF
      bool _; // no data needed
    } power_off;
  } data;

} sbc_task_queue_data_t;
#define SBC_TASK_QUEUE_LENGTH 4
#define SBC_TASK_QUEUE_ITEM_SIZE sizeof(sbc_task_queue_data_t)
static QueueHandle_t s_sbc_task_queue_hd;
static uint8_t s_sbc_queue_buff[SBC_TASK_QUEUE_LENGTH * SBC_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_sbc_queue_struct;

#define SBC_TASK_SIZE (4096)
static TaskHandle_t s_sbc_task_hd = NULL;
static StackType_t s_sbc_task_buff[SBC_TASK_SIZE];
static StaticTask_t s_sbc_task_struct;

mw_sbc_ipv4_change_callback ipv4_change_callback = NULL;
mw_sbc_connection_change_callback connection_change_callback = NULL;

static uint32_t s_last_ip = 0;

#define BUFF_MAX 50
static void s_process_tx(sbc_task_queue_data_t *queue_data) {
  char buff[BUFF_MAX];

  switch (queue_data->command) {
  case SBC_TASK_COMMAND_BATTERY_INFO:
    sprintf(buff, "$BT,%.2f,%.2f\r\n", queue_data->data.battery_info.voltage, queue_data->data.battery_info.percentage);
    if (tud_cdc_n_connected(1)) {
      tud_cdc_n_write_str(1, buff);
      tud_cdc_n_write_flush(1);
    }
    break;

  case SBC_TASK_COMMAND_POWER_OFF:
    sprintf(buff, "$PO,0\r\n");
    if (tud_cdc_n_connected(1)) {
      tud_cdc_n_write_str(1, buff);
      tud_cdc_n_write_flush(1);
    }
    break;

  case SBC_TASK_COMMAND_NONE:
  default:
    break;
  }
}

#define SERIALIZE(x) (x[0] << 8 | x[1])
static bool s_parse(char *arr) {
  uint16_t command = arr[1] << 8 | arr[2];
  switch (command) {
  case 0x4950: { // IP
    uint32_t curr_ip = 0;
    int ip_1, ip_2, ip_3, ip_4;

    int ret = sscanf(arr, "$IP,%d,%d,%d,%d", &ip_1, &ip_2, &ip_3, &ip_4);
    if (ret != 4) {
      return false;
    }

    curr_ip = (uint8_t)ip_1 << 24 | (uint8_t)ip_2 << 16 | (uint8_t)ip_3 << 8 | (uint8_t)ip_4;

    if (s_last_ip != curr_ip) {
      log_debug(TAG, "IP Changed: %d.%d.%d.%d -> %d.%d.%d.%d",
                (int)(s_last_ip >> 24), (int)(s_last_ip >> 16 & 0b11111111), (int)(s_last_ip >> 8 & 0b11111111), (int)(s_last_ip & 0b11111111),
                ip_1, ip_2, ip_3, ip_4);

      s_last_ip = curr_ip;
      if (ipv4_change_callback != NULL) {
        ipv4_change_callback(curr_ip);
      }
    }
    break;
  }

  default:
    return false;
  }

  return true;
}

static void s_process_rx(void) {
  static uint8_t i = 0;
  static char buff[BUFF_MAX] = {0};

  while (tud_cdc_n_available(1) != 0) {
    char c = tud_cdc_n_read_char(1);
    if (i == 0 && c != '$') {
      continue;
    }
    buff[i] = c;

    if (i >= 1 && buff[i - 1] == '\r' && buff[i] == '\n') {
      buff[i + 1] = 0;

      s_parse(buff);
      i = 0;

    } else {
      i++;
      if (i >= BUFF_MAX - 2) { // overflow
        i = 0;
      }
    }
  }
}

static void s_process_connection_change_event(void) {
  static mw_sbc_connection_status_t last_connection_status = MW_SBC_CONNECTION_NONE;
  mw_sbc_connection_status_t current_connection_status;

  bool temp_usb = tud_ready();
  bool temp_cdc = tud_cdc_n_connected(0) || tud_cdc_n_connected(1);
  if (temp_cdc == true) {
    current_connection_status = MW_SBC_CONNECTION_CDC;
  } else if (temp_usb == true) {
    current_connection_status = MW_SBC_CONNECTION_USB;
  } else {
    current_connection_status = MW_SBC_CONNECTION_NONE;
  }

  if (last_connection_status != current_connection_status) {
    if (connection_change_callback != NULL) {
      last_connection_status = current_connection_status; // event must be processed
      connection_change_callback(current_connection_status);
    }

    if (current_connection_status == MW_SBC_CONNECTION_NONE) {
      s_last_ip = 0;
      if (ipv4_change_callback != NULL) {
        ipv4_change_callback(0);
      }
    }
  }
}

static void s_sbc_task(void *arg) {
  (void)arg;

  sbc_task_queue_data_t queue_data = {0};

  TickType_t last_connection_check_tick = xTaskGetTickCount();

  tud_cdc_n_read_flush(1);
  tud_cdc_n_write_clear(1);

  for (;;) {
    // process tx
    if (xQueueReceive(s_sbc_task_queue_hd, &queue_data, pdMS_TO_TICKS(10)) == pdTRUE) {
      s_process_tx(&queue_data);
    }

    // process rx
    s_process_rx();

    // connection change event
    if (xTaskGetTickCount() - last_connection_check_tick >= pdMS_TO_TICKS(500)) {
      last_connection_check_tick = xTaskGetTickCount();
      s_process_connection_change_event();
    }
  }
}

bool mw_sbc_init(void) {
  s_sbc_task_queue_hd = xQueueCreateStatic(
      SBC_TASK_QUEUE_LENGTH,
      SBC_TASK_QUEUE_ITEM_SIZE,
      s_sbc_queue_buff,
      &s_sbc_queue_struct);

  s_sbc_task_hd = xTaskCreateStatic(
      s_sbc_task,
      "sbc",
      SBC_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES - 2,
      s_sbc_task_buff,
      &s_sbc_task_struct);

  return true;
}

static bool s_send_queue(sbc_task_queue_data_t *queue_data) {
  bool ret = xQueueSend(s_sbc_task_queue_hd, queue_data, 0) == pdTRUE;
  if (ret == false) {
    log_warning(TAG, "Publish queue full!! message dropped!!");
  }

  return ret;
}

bool mw_sbc_report_battery_info(float voltage, float percentage) {
  sbc_task_queue_data_t queue_data;
  queue_data.command = SBC_TASK_COMMAND_BATTERY_INFO;
  queue_data.data.battery_info.voltage = voltage;
  queue_data.data.battery_info.percentage = percentage;

  return s_send_queue(&queue_data);
}

bool mw_sbc_report_power_off(void) {
  sbc_task_queue_data_t queue_data;
  queue_data.command = SBC_TASK_COMMAND_POWER_OFF;

  return s_send_queue(&queue_data);
}

void mw_sbc_set_ipv4_change_callback(mw_sbc_ipv4_change_callback callback) {
  ipv4_change_callback = callback;
}

void mw_sbc_set_connection_change_callback(mw_sbc_connection_change_callback callback) {
  connection_change_callback = callback;
}
