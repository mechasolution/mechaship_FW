#ifndef SRC_HW_HAL_TIME_TIME
#define SRC_HW_HAL_TIME_TIME

#include <stdbool.h>
#include <stdint.h>

bool time_init(void);

uint32_t time_get_millis(void);
uint64_t time_get_micros(void);
void time_block_ms(uint32_t ms);
void time_block_us(uint64_t us);

static inline void time_block_ns(uint32_t ns) {
  if (ns == 0U) {
    return;
  }

  /* Zephyr exposes microsecond busy wait; round up for tiny delays. */
  uint32_t us = (ns + 999U) / 1000U;
  if (us == 0U) {
    us = 1U;
  }
  time_block_us(us);
}

#endif /* SRC_HW_HAL_TIME_TIME */
