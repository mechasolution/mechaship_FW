#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <pico/critical_section.h>

#include <hardware/gpio.h>
#include <hardware/uart.h>

#include "hal/time/time.h"

#include "log.h"

static critical_section_t s_uart_lock;
static log_level_t s_log_level = LOG_NONE;

bool log_init(void) {
  uart_init(HWCONF_SERIAL_DEBUG_ID, HWCONF_SERIAL_DEBUG_BAUDRATE);

  gpio_set_function(HWCONF_SERIAL_DEBUG_PIN_TX, UART_FUNCSEL_NUM(HWCONF_SERIAL_DEBUG_ID, HWCONF_SERIAL_DEBUG_PIN_TX));
  gpio_set_function(HWCONF_SERIAL_DEBUG_PIN_RX, UART_FUNCSEL_NUM(HWCONF_SERIAL_DEBUG_ID, HWCONF_SERIAL_DEBUG_PIN_RX));

  critical_section_init(&s_uart_lock);

  return true;
}

void log_set_level(log_level_t target) {
  s_log_level = target;
}

static void s_log(log_level_t level, const char *tag, const char *format, va_list args) {
  if (level < s_log_level || level > LOG_MAX) {
    return;
  }

  const char *color = "\e[0m";
  char label = '?';
  switch (level) {
  case LOG_DEBUG: color = "\e[90m"; label = 'D'; break;
  case LOG_INFO: color = "\e[32m"; label = 'I'; break;
  case LOG_WARNING: color = "\e[33m"; label = 'W'; break;
  case LOG_ERROR: color = "\e[31m"; label = 'E'; break;
  default: return;
  }

  char line[320];
  int prefix_len = snprintf(line, sizeof(line), "%s%c (%u) %s: ", color, label, time_get_millis(), tag);
  if (prefix_len < 0) return;
  size_t used = (size_t)prefix_len;
  if (used > sizeof(line) - 7) used = sizeof(line) - 7;
  line[used] = '\0';
  vsnprintf(line + used, sizeof(line) - used - 6, format, args);
  used += strlen(line + used);
  snprintf(line + used, sizeof(line) - used, "\e[0m\r\n");

  critical_section_enter_blocking(&s_uart_lock);
  uart_puts(HWCONF_SERIAL_DEBUG_ID, line);
  critical_section_exit(&s_uart_lock);
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
