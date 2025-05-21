#ifndef F8690D39_57DF_442E_93A0_DDBA62F3F4F0_H_
#define F8690D39_57DF_442E_93A0_DDBA62F3F4F0_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  SLED_TASK_PATTERN_OFF = 0,
  SLED_TASK_PATTERN_SOLID,

  SLED_TASK_PATTERN_BLINK_SLOW,
  SLED_TASK_PATTERN_BLINK_FAST,

  SLED_TASK_PATTERN_FLASH,
  SLED_TASK_PATTERN_REFLASH,

  SLED_TASK_PATTERN_MAX,
} sled_task_pattern_t;

bool sled_task_init(void);

bool sled_task_set_pattern(sled_task_pattern_t pattern);

#endif /* F8690D39_57DF_442E_93A0_DDBA62F3F4F0_H_ */
