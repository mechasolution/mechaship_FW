#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <Arduino.h>

#include "../bsp/bsp.h"
#include "hw.h"
#include "include/error_loop.h"

extern bsp_neopixel_handler_t neopixel_h;

void error_loop(rcl_ret_t error, const char *file_name_, const char *function_name_, int line_cnt_) {
  pinMode(HW_PIN_STATUS_LED, OUTPUT);
  char buff[200];
  sprintf(buff, "ERRORTYPE: %ld, FILENAME: %s, FUNCTION: %s, LINENO: %d", error, file_name_, function_name_, line_cnt_);
  unsigned long reset_time = millis() + 5000;
  while (1) {
    if (reset_time <= millis()) {
      digitalWrite(HW_PIN_RESET, LOW);
    }
    HW_SERIAL_DEBUG.println(buff);
    digitalWrite(HW_PIN_STATUS_LED, HIGH);
    bsp_neopixel_set(&neopixel_h, 255, 0, 0, 0);
    delay(100);
    digitalWrite(HW_PIN_STATUS_LED, LOW);
    bsp_neopixel_set(&neopixel_h, 0, 0, 0, 0);
    delay(100);
  }
}
