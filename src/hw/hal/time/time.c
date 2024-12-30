#include "pico/time.h"

#include "time.h"

bool time_init(void) {
  /* nothing to do */

  return true;
}

uint32_t time_get_millis(void) {
  return to_ms_since_boot(get_absolute_time());
}

uint64_t time_get_micros(void) {
  return to_us_since_boot(get_absolute_time());
}

void time_block_ms(uint32_t ms) {
  sleep_ms(ms);
}

void time_block_us(uint64_t us) {
  sleep_us(us);
}
