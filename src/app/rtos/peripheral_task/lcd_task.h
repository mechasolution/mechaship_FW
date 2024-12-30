#ifndef EE134825_9CDE_4665_A9A9_F257B86108DB_H_
#define EE134825_9CDE_4665_A9A9_F257B86108DB_H_

#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <queue.h>

typedef struct {
  enum {
    LCD_TASK_COMMAND_NONE = 0x00,

    LCD_TASK_COMMAND_CONNECTION,
    LCD_TASK_COMMAND_ACT_POWER,
    LCD_TASK_COMMAND_IP_ADDR,
    LCD_TASK_COMMAND_ACT_STATUS,
    LCD_TASK_COMMAND_BAT_STATUS,
    LCD_TASK_COMMAND_POWER_OFF,
  } command;
  union {
    struct {
      bool status;
    } connection;

    struct {
      bool status;
    } act_power;

    struct {
      uint32_t ipv4;
    } ip_addr;

    struct {
      uint8_t key;
      uint8_t throttle;
    } act_status;

    struct {
      uint8_t value;
    } bat_status;

    struct {
      uint8_t countdown;
      bool is_low_power;
    } power_off;
  } data;
} lcd_task_queue_data_t;

extern QueueHandle_t lcd_task_queue_hd;
extern TaskHandle_t lcd_task_hd;

#endif /* EE134825_9CDE_4665_A9A9_F257B86108DB_H_ */
