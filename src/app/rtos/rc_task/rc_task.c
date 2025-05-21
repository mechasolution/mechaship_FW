#include <math.h>

#include <FreeRTOS.h>
#include <task.h>

#include "rtos/peripheral_task/actuator_task.h"
#include "rtos/peripheral_task/lcd_task.h"
#include "rtos/peripheral_task/sled_task.h"

#include "driver/log/log.h"
#include "driver/power/power.h"
#include "driver/rc4/rc4.h"

#include "rc_task.h"

#include "hwconf.h"

#define TAG "rc"

#define RC_TASK_SIZE configMINIMAL_STACK_SIZE
static TaskHandle_t s_rc_task_hd = NULL;
static StackType_t s_rc_task_buff[RC_TASK_SIZE];
static StaticTask_t s_rc_task_struct;

static void s_update_lcd(float throttle, float key) {
  static TickType_t last_update = 0;

  if (xTaskGetTickCount() - last_update >= pdMS_TO_TICKS(100)) {
    last_update = xTaskGetTickCount();
    lcd_task_update_key((uint8_t)key);
    lcd_task_update_throttle((int8_t)throttle);
  }
}

static void s_rander_color(void) {
  enum { RG,
         GB,
         BR,
  } static color_phase = RG;
  static uint8_t step = 1;
  static uint8_t r = 255, g = 0, b = 0;
  static uint8_t cnt = 0;

  if (cnt < 20) { // 20ms period
    cnt++;
    return;
  }
  cnt = 0;

  switch (color_phase) {
  case RG:
    if (g < 255)
      g += step;
    else
      color_phase = GB;
    break;
  case GB:
    if (r > 0)
      r -= step;
    else
      color_phase = BR;
    break;
  case BR:
    if (b < 255) {
      b += step;
    } else if (g > 0) {
      g -= step;
    } else if (r < 255) {
      r += step;
    } else {
      color_phase = RG;
    }
    break;
  }
  actuator_task_set_rgbwled(r, g, b, 0);
}

static void s_rc_task(void *arg) {
  bool switch_status = rc4_get_switch();
  sled_task_set_pattern(SLED_TASK_PATTERN_SOLID);

  actuator_task_set_tone(523, 150);
  actuator_task_set_tone(622, 150);
  actuator_task_set_tone(739, 150);
  actuator_task_set_power(true, 0, 180, 500, 2500, 1500, 200);
  actuator_task_set_rgbwled(0, 0, 0, 0);

  for (;;) {
    float throttle = rc4_get_throttle_percentage();
    float key = rc4_get_key_degree();
    actuator_task_set_throttle(throttle);
    actuator_task_set_key(key);

    s_update_lcd(throttle, key);

    bool temp = rc4_get_switch();
    if (switch_status != temp) {
      switch_status = temp;
      if (switch_status == false) {
        vTaskDelay(pdMS_TO_TICKS(10)); // wait until rgbwled set command executed (TODO: use dma on rgbwled driver)
        actuator_task_set_rgbwled(0, 0, 0, 0);
      }
    }
    if (switch_status == true) {
      s_rander_color();
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

bool rc_task_init(void) {
  if (s_rc_task_hd != NULL) {
    return false;
  }

  s_rc_task_hd = xTaskCreateStatic(
      s_rc_task,
      "rc",
      RC_TASK_SIZE,
      NULL,
      configIDLE_TASK_PRIORITIES,
      s_rc_task_buff,
      &s_rc_task_struct);

  return true;
}

bool rc_task_deinit(void) {
  if (s_rc_task_hd == NULL) {
    // not created
    return true;
  }

  eTaskState task_status = eTaskGetState(s_rc_task_hd);
  vTaskDelete(s_rc_task_hd);
  s_rc_task_hd = NULL;

  vTaskDelay(pdMS_TO_TICKS(10)); // wait until rgbwled set command executed (TODO: use dma on rgbwled driver)

  actuator_task_set_power(false, 0.0, 0.0, 0.0, 0, 0, 0);
  actuator_task_set_rgbwled(0, 0, 0, 0);

  return true;
}
