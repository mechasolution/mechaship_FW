#include <zephyr/kernel.h>

#include "time.h"

bool time_init(void) {
  return true;
}

uint32_t time_get_millis(void) {
  return (uint32_t)k_uptime_get();
}

uint64_t time_get_micros(void) {
  return k_ticks_to_us_floor64(k_uptime_ticks());
}

void time_block_ms(uint32_t ms) {
  k_msleep(ms);
}

void time_block_us(uint64_t us) {
  k_busy_wait((uint32_t)us);
}
