#include <stdio.h>
#include <time.h>

#include "hal/time/time.h"

#include <FreeRTOS.h>
#include <task.h>

#include <tusb.h>

#include <uxr/client/profile/transport/custom/custom_transport.h>

void usleep(uint64_t us) {
  time_block_us(us);
}

int clock_gettime(clockid_t unused, struct timespec *tp) {
  uint64_t m = time_get_micros();
  tp->tv_sec = m / 1000000;
  tp->tv_nsec = (m % 1000000) * 1000;
  return 0;
}

bool cdc0_transport_open(struct uxrCustomTransport *transport) {
  /* nothing to do */

  return true;
}

bool cdc0_transport_close(struct uxrCustomTransport *transport) {
  /* nothing to do */

  return true;
}

size_t cdc0_transport_write(struct uxrCustomTransport *transport, uint8_t *buf, size_t len, uint8_t *errcode) {
  uint32_t len_sent = tud_cdc_n_write(0, buf, len);
  tud_cdc_n_write_flush(0);

  if (len_sent != len_sent) {
    *errcode = 1;
  }

  return len_sent;
}

size_t cdc0_transport_read(struct uxrCustomTransport *transport, uint8_t *buf, size_t len, int timeout, uint8_t *errcode) {
  uint64_t start_time_us = time_get_micros();
  uint64_t timeout_us = timeout * 1000;

  for (size_t i = 0; i < len; i++) {
    while (tud_cdc_n_available(0) == 0) {
      if (time_get_micros() - start_time_us > timeout_us) {
        *errcode = 1;
        return i;
      }
      vTaskDelay(pdMS_TO_TICKS(1));
    }

    buf[i] = tud_cdc_n_read_char(0);
  }

  return len;
}