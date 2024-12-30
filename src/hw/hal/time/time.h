#ifndef SRC_HW_HAL_TIME_TIME
#define SRC_HW_HAL_TIME_TIME

#include <stdbool.h>
#include <stdint.h>

bool time_init(void);

uint32_t time_get_millis(void);
uint64_t time_get_micros(void);
void time_block_ms(uint32_t ms);
void time_block_us(uint64_t us);
inline void time_block_ns(uint32_t ns_multi_by_8) {
  volatile uint32_t count = ns_multi_by_8;
  while (count--) {
    __asm volatile("nop");
  }
}

#endif /* SRC_HW_HAL_TIME_TIME */
