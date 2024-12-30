#ifndef SRC_HW_DRIVER_LOG_CONSOLE
#define SRC_HW_DRIVER_LOG_CONSOLE

#include <stdbool.h>

#define LOG_RETURN_FALSE(TAG, fn)                                                    \
  {                                                                                  \
    if ((fn != true)) {                                                              \
      log_error(TAG, "RETURN FALSE!! %s, %s, %d", __FILE__, __FUNCTION__, __LINE__); \
      return false;                                                                  \
    }                                                                                \
  }

typedef enum {
  LOG_NONE = 0x00,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,

  LOG_MAX,
} log_level_t;

bool log_init(void);

void log_set_level(log_level_t target);
void log_debug(const char tag[], const char format[], ...);
void log_info(const char tag[], const char format[], ...);
void log_warning(const char tag[], const char format[], ...);
void log_error(const char tag[], const char format[], ...);

#endif /* SRC_HW_DRIVER_LOG_CONSOLE */
