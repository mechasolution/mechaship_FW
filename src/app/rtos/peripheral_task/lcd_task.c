#include <stdio.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "driver/lcd/lcd.h"
#include "driver/log/log.h"

#include "hal/time/time.h"

#include "lcd_task.h"

#define TAG "lcd_task"

#define LCD_TASK_SIZE configMINIMAL_STACK_SIZE
TaskHandle_t lcd_task_hd = NULL;
static StackType_t s_lcd_task_buff[LCD_TASK_SIZE];
static StaticTask_t s_lcd_task_struct;

#define LCD_TASK_QUEUE_LENGTH 10
#define LCD_TASK_QUEUE_ITEM_SIZE sizeof(lcd_task_queue_data_t)
QueueHandle_t lcd_task_queue_hd = NULL;
static uint8_t s_lcd_task_queue_buff[LCD_TASK_QUEUE_LENGTH * LCD_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_lcd_task_queue_struct;

// internal
#define LCD_FRAME_TASK_SIZE configMINIMAL_STACK_SIZE
static TaskHandle_t s_lcd_frame_task_hd = NULL;
static StackType_t s_lcd_frame_task_buff[LCD_FRAME_TASK_SIZE];
static StaticTask_t s_lcd_frame_task_struct;

static void s_lcd_frame_task(void *arg) {
  lcd_task_queue_data_t data = {0};
  data.command = LCD_TASK_COMMAND_FRAME;

  for (;;) {
    vTaskDelay((500 * 1) / portTICK_PERIOD_MS);

    xQueueSend(lcd_task_queue_hd, &data, 0);
  }
}

static void s_lcd_task(void *arg) {
  (void)arg;

  lcd_task_queue_data_t lcd_queue_data = {0};
  char line_buff[16 + 1];

  lcd_set_cursor(0, 0);
  lcd_set_string("SBC Disconnected");
  lcd_set_cursor(1, 0);
  lcd_set_string("ACT OFF     B   ");

  for (;;) {
    if (xQueueReceive(lcd_task_queue_hd, &lcd_queue_data, portMAX_DELAY) == pdTRUE) {
      switch (lcd_queue_data.command) {
      case LCD_TASK_COMMAND_CONNECTION:
        lcd_set_cursor(0, 0);
        if (lcd_queue_data.data.connection.status) {
          lcd_set_string("SBC Connected   ");
          vTaskDelay(1000);
          lcd_set_cursor(0, 0);
          lcd_set_string("Waiting IP Addr ");
        } else {
          lcd_set_string("SBC Disconnected");
          vTaskDelay(1000);
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
        if (lcd_queue_data.data.ip_addr.ipv4 == 0) {
          sprintf(line_buff, "Waiting IP Addr ");
        } else {
          uint8_t octet1 = (lcd_queue_data.data.ip_addr.ipv4 >> 24) & 0xFF;
          uint8_t octet2 = (lcd_queue_data.data.ip_addr.ipv4 >> 16) & 0xFF;
          uint8_t octet3 = (lcd_queue_data.data.ip_addr.ipv4 >> 8) & 0xFF;
          uint8_t octet4 = lcd_queue_data.data.ip_addr.ipv4 & 0xFF;

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
        lcd_set_cursor(0, 0);
        lcd_set_string(line_buff);

        break;

      case LCD_TASK_COMMAND_ACT_STATUS:
        if (lcd_queue_data.data.act_status.throttle == 0) {
          sprintf(line_buff, "T OFF K%3d  ",
                  lcd_queue_data.data.act_status.key);
        } else if (lcd_queue_data.data.act_status.throttle > 0) {
          sprintf(line_buff, "T+%3d K%3d  ",
                  lcd_queue_data.data.act_status.throttle,
                  lcd_queue_data.data.act_status.key);
        } else {
          lcd_queue_data.data.act_status.throttle *= -1;
          sprintf(line_buff, "T-%3d K%3d  ",
                  lcd_queue_data.data.act_status.throttle,
                  lcd_queue_data.data.act_status.key);
        }
        lcd_set_cursor(1, 0);
        lcd_set_string(line_buff);
        break;

      case LCD_TASK_COMMAND_BAT_STATUS:
        sprintf(line_buff, "B%3d",
                lcd_queue_data.data.bat_status.value);
        lcd_set_cursor(1, 12);
        lcd_set_string(line_buff);
        break;

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

      case LCD_TASK_COMMAND_NONE:
      default:
        break;
      }
    }
  }
}

bool lcd_task_init(void) {
  lcd_task_hd = xTaskCreateStatic(
      s_lcd_task,
      "lcd",
      LCD_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES,
      s_lcd_task_buff,
      &s_lcd_task_struct);

  s_lcd_frame_task_hd = xTaskCreateStatic(
      s_lcd_frame_task,
      "lcd_init",
      LCD_FRAME_TASK_SIZE,
      NULL,
      1,
      s_lcd_frame_task_buff,
      &s_lcd_frame_task_struct);

  return true;
}

bool lcd_task_queue_init(void) {
  lcd_task_queue_hd = xQueueCreateStatic(
      LCD_TASK_QUEUE_LENGTH,
      LCD_TASK_QUEUE_ITEM_SIZE,
      s_lcd_task_queue_buff,
      &s_lcd_task_queue_struct);

  return true;
}