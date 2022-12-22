#include <stdbool.h>

#include <Arduino.h>

#include "hw.h"
#include "include/error_loop.h"

void (*reset_func)(void) = 0;

void error_loop(rcl_ret_t error, const char *file_name_, const char *function_name_, int line_cnt_) {
  pinMode(HW_PIN_STATUS_LED, OUTPUT);
  char *buff;
  asprintf(&buff, "ERRORTYPE: %d, FILENAME: %s, FUNCTION: %s, LINENO: %d", error, file_name_, function_name_, line_cnt_);
  unsigned long reset_time = millis() + 10000;
  while (1) {
    if (reset_time <= millis()) {
      reset_func();
    }
    HW_SERIAL_DEBUG.println(buff);
    digitalWrite(HW_PIN_STATUS_LED, HIGH);
    delay(100);
    digitalWrite(HW_PIN_STATUS_LED, LOW);
    delay(100);
  }
}
