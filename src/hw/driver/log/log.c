#include <stdarg.h>
#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "hal/time/time.h"
#include "log.h"

static struct k_mutex s_uart_lock;
static log_level_t s_log_level = LOG_NONE;

bool log_init(void) {
  k_mutex_init(&s_uart_lock);
  return true;
}

void log_set_level(log_level_t target) {
  s_log_level = target;
}

static inline void s_print_string(const char *s) {
  k_mutex_lock(&s_uart_lock, K_FOREVER);
  printk("%s", s);
  k_mutex_unlock(&s_uart_lock);
}

static void s_log(log_level_t level, const char *tag, const char *format, va_list args) {
  if (level < s_log_level || level > LOG_MAX) {
    return;
  }

  switch (level) {
  case LOG_DEBUG:
    s_print_string("\e[90mD (");
    break;
  case LOG_INFO:
    s_print_string("\e[32mI (");
    break;
  case LOG_WARNING:
    s_print_string("\e[33mW (");
    break;
  case LOG_ERROR:
    s_print_string("\e[31mE (");
    break;
  case LOG_NONE:
  case LOG_MAX:
  default:
    break;
  }

  char buffer[256];
  snprintf(buffer, sizeof(buffer), "%u", time_get_millis());
  s_print_string(buffer);
  s_print_string(") ");
  s_print_string(tag);
  s_print_string(": ");

  vsnprintf(buffer, sizeof(buffer), format, args);
  s_print_string(buffer);
  s_print_string("\e[0m\r\n");
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
