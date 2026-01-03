#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include <tusb.h>

#include "tusb_.h"

#define TUSB_TASK_SIZE (4096)
static TaskHandle_t s_tusb_task_hd = NULL;
static StackType_t s_tusb_task_buff[TUSB_TASK_SIZE];
static StaticTask_t s_tusb_task_struct;

static TimerHandle_t s_tusb_status_check_timer_hd = NULL;
static StaticTimer_t s_tusb_status_check_timer_buff;

static tusb_cdc0_rx_cb s_cdc0_rx_cb = NULL;
static tusb_cdc1_rx_cb s_cdc1_rx_cb = NULL;
static tusb_connection_change_cb s_connection_change_cb = NULL;

static tusb_connection_t s_last_conn_status = TUSB_CONNECTION_NONE;

static void s_tusb_status_check_timer_callback(TimerHandle_t timer_hd) {
  bool temp_usb = tud_ready();
  bool temp_cdc = tud_cdc_n_connected(0) || tud_cdc_n_connected(1);
  tusb_connection_t conn_status;
  if (temp_cdc == true) {
    conn_status = TUSB_CONNECTION_CDC;
  } else if (temp_usb == true) {
    conn_status = TUSB_CONNECTION_USB;
  } else {
    conn_status = TUSB_CONNECTION_NONE;
  }

  if (s_last_conn_status != conn_status && s_connection_change_cb != NULL) {
    s_connection_change_cb(conn_status);
  }

  s_last_conn_status = conn_status;
}

void tud_cdc_rx_cb(uint8_t itf) {
  switch (itf) {
  case 0:
    if (s_cdc0_rx_cb != NULL) {
      s_cdc0_rx_cb();
    }
    break;

  case 1:
    if (s_cdc1_rx_cb != NULL) {
      s_cdc1_rx_cb();
    }

  default:
    break;
  }
}

static void s_tusb_task(void *arg) {
  (void)arg;

  tusb_init();

  xTimerStart(s_tusb_status_check_timer_hd, 0);

  for (;;) {
    tud_task();
  }
}

bool mw_tusb_init(void) {
  s_tusb_status_check_timer_hd = xTimerCreateStatic(
      "status_check_timer",
      pdMS_TO_TICKS(500),
      pdTRUE,
      (void *)0,
      s_tusb_status_check_timer_callback,
      &s_tusb_status_check_timer_buff);

  s_tusb_task_hd = xTaskCreateStatic(
      s_tusb_task,
      "tusb",
      TUSB_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES - 1,
      s_tusb_task_buff,
      &s_tusb_task_struct);

  return s_tusb_task_hd != NULL;
}

void mw_tusb_set_cdc0_callback(tusb_cdc0_rx_cb cb) {
  s_cdc0_rx_cb = cb;
}

void mw_tusb_set_cdc1_callback(tusb_cdc1_rx_cb cb) {
  s_cdc1_rx_cb = cb;
}

void mw_tusb_set_connection_change_callback(tusb_connection_change_cb cb) {
  s_connection_change_cb = cb;
}