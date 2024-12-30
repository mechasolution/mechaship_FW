#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

#include "rtos/central_task/central_task.h"
#include "rtos/peripheral_task/actuator_task.h"
#include "rtos/peripheral_task/lcd_task.h"
#include "rtos/uros_task/uros_task.h"

#include "driver/actuator/actuator.h"
#include "driver/battery/battery.h"
#include "driver/emo/emo.h"
#include "driver/led/led.h"
#include "driver/log/log.h"
#include "driver/power/power.h"

#include "hal/time/time.h"

#define TAG "sensor"

#define TASK_SIZE configMINIMAL_STACK_SIZE
static StackType_t sensor_task_buff[TASK_SIZE];
static StaticTask_t sensor_task_struct;

static void s_sensor_task(void *arg) {
  (void)arg;

  lcd_task_queue_data_t lcd_queue_data;
  uros_task_pub_queue_data_t uros_pub_queue_data;
  central_task_queue_data_t central_queue_data;
  power_button_status_t button_status;

  for (;;) {
    // send actuator, battery status to lcd task
    if (power_get_act()) {
      lcd_queue_data.command = LCD_TASK_COMMAND_ACT_STATUS;
      lcd_queue_data.data.act_status.key = (uint8_t)roundf(actuator_get_key_degree());
      lcd_queue_data.data.act_status.throttle = (uint8_t)roundf(actuator_get_thruster_percentage());
      xQueueSend(lcd_task_queue_hd, &lcd_queue_data, 0);
    }
    lcd_queue_data.command = LCD_TASK_COMMAND_BAT_STATUS;
    lcd_queue_data.data.bat_status.value = (uint8_t)battery_get_percentage();
    xQueueSend(lcd_task_queue_hd, &lcd_queue_data, 0);

    // check battery percentage
    if (battery_get_percentage() <= 0) {
      central_queue_data.event_type = CENTRAL_TASK_EVENT_LOW_POWER;
      central_queue_data.data.low_power.voltage = battery_get_voltage();
      xQueueSend(central_task_queue_hd, &central_queue_data, 0);

      vTaskSuspend(NULL); // stop this task
    }

    // send low frequency sensor data to uros task (to publish)
    uros_pub_queue_data.topic = UROS_TASK_PUB_LOW_FREQ_SENSOR_DATA;
    uros_pub_queue_data.data.low_freq_sensor_data.battery_voltage = battery_get_voltage();
    uros_pub_queue_data.data.low_freq_sensor_data.emo_status = emo_get_status();
    xQueueSend(uros_task_pub_queue_hd, &uros_pub_queue_data, 0);

    // check power status & send power off event to central task
    power_get_button_status(&button_status);
    if (button_status.status && time_get_millis() - button_status.last_change_timestamp > 4000) {
      central_queue_data.event_type = CENTRAL_TASK_EVENT_POWER_OFF;
      xQueueSend(central_task_queue_hd, &central_queue_data, portMAX_DELAY);

      vTaskSuspend(NULL); // stop this task
    }

    vTaskDelay(1000);
  }
}

bool sensor_task_init(void) {
  xTaskCreateStatic(
      s_sensor_task,
      "sensor",
      TASK_SIZE,
      NULL,
      1,
      sensor_task_buff,
      &sensor_task_struct);

  return true;
}
