#ifndef EE134825_9CDE_4665_A9A9_F257B86108DB_H_
#define EE134825_9CDE_4665_A9A9_F257B86108DB_H_

#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <queue.h>

typedef enum {
  LCD_TASK_CTL_MODE_NONE = 1,
  LCD_TASK_CTL_MODE_RC,
  LCD_TASK_CTL_MODE_ROS,
} lcd_task_ctl_mode_t;

bool lcd_task_init(void);

bool lcd_task_update_ctl_mode(lcd_task_ctl_mode_t mode);
bool lcd_task_update_actuator_power(bool status);
bool lcd_task_update_ip_addr(uint32_t ipv4);
bool lcd_task_update_throttle(int8_t percentage);
bool lcd_task_update_key(uint8_t degree);
bool lcd_task_update_battery(uint8_t percentage);
bool lcd_task_update_power_off(uint8_t countdown, bool is_low_power);

void lcd_task_noti_usb_unplugged(void);
void lcd_task_noti_usb_plugged(void);
void lcd_task_noti_cdc_connected(void);
void lcd_task_noti_uros_connected(void);
void lcd_task_noti_uros_disconnected(void);

#endif /* EE134825_9CDE_4665_A9A9_F257B86108DB_H_ */
