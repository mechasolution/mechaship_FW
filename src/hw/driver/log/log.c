#include <stdarg.h>
#include <stdio.h>

#include <hardware/gpio.h>
#include <hardware/uart.h>

#include "hal/time/time.h"

#include "log.h"

static log_level_t s_log_level = LOG_NONE;

bool log_init(void) {
  uart_init(HWCONF_SERIAL_DEBUG_ID, HWCONF_SERIAL_DEBUG_BAUDRATE);

  gpio_set_function(HWCONF_SERIAL_DEBUG_PIN_TX, UART_FUNCSEL_NUM(HWCONF_SERIAL_DEBUG_ID, HWCONF_SERIAL_DEBUG_PIN_TX));
  gpio_set_function(HWCONF_SERIAL_DEBUG_PIN_RX, UART_FUNCSEL_NUM(HWCONF_SERIAL_DEBUG_ID, HWCONF_SERIAL_DEBUG_PIN_RX));

  return true;
}

void log_set_level(log_level_t target) {
  s_log_level = target;
}

static inline void s_print_string(const char *s) {
  uart_puts(HWCONF_SERIAL_DEBUG_ID, s);
}

static void s_log(log_level_t level, const char *tag, const char *format, va_list args) {
  if (level < s_log_level || level > LOG_MAX) {
    return;
  }

  switch (level) {
  case LOG_DEBUG:
    s_print_string("\e[90m"); // 회색
    s_print_string("D (");
    break;

  case LOG_INFO:
    s_print_string("\e[32m"); // 초록색
    s_print_string("I (");
    break;

  case LOG_WARNING:
    s_print_string("\e[33m"); // 주황색
    s_print_string("W (");
    break;

  case LOG_ERROR:
    s_print_string("\e[31m"); // 빨간색
    s_print_string("E (");
    break;

  case LOG_NONE:
  case LOG_MAX:
  default:
    break;
  }

  char buffer[256];
  sprintf(buffer, "%u", time_get_millis());
  s_print_string(buffer);
  s_print_string(") ");
  s_print_string(tag);
  s_print_string(": ");

  vsnprintf(buffer, sizeof(buffer), format, args);
  s_print_string(buffer);
  s_print_string("\e[0m");
  s_print_string("\r\n");
}

void log_debug(const char *tag, const char *format, ...) {
  va_list args;
  va_start(args, format);
  s_log(LOG_DEBUG, tag, format, args);
  va_end(args);
}

void log_info(const char *tag, const char *format, ...) {
  va_list args;
  va_start(args, format);
  s_log(LOG_INFO, tag, format, args);
  va_end(args);
}

void log_warning(const char *tag, const char *format, ...) {
  va_list args;
  va_start(args, format);
  s_log(LOG_WARNING, tag, format, args);
  va_end(args);
}

void log_error(const char *tag, const char *format, ...) {
  va_list args;
  va_start(args, format);
  s_log(LOG_ERROR, tag, format, args);
  va_end(args);
}
