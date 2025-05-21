#include <stdio.h>

#include "hardware/pio.h"

#include "hal/time/time.h"

#include "hwconf.h"
#include "pwm_capture.pio.h"
#include "rc4.h"

#define MAX_CHANNEL NUM_PIO_STATE_MACHINES
#define PIO_CHANNEL HWCONF_RC_PIO_ID
#define TIMEOUT_MS 50 // 50Hz period=20ms

typedef struct {
  uint32_t pulse_width_us;
  uint32_t last_update_time_ms;
} rc4_channel_data_t;
static rc4_channel_data_t s_channel_data[4];
static rc4_slideswitch_data_t s_slideswitch_last = RC4_SLIDESWITCH_MIDDLE;
static bool s_switch_last = false;

static void s_pio_irq_func(void) {
  for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
    if (pio_interrupt_get(PIO_CHANNEL, i)) {
      uint32_t pulsewidth, period;
      pulsewidth = pio_sm_get(PIO_CHANNEL, i);
      period = pio_sm_get(PIO_CHANNEL, i) + pulsewidth;
      pulsewidth = 2 * pulsewidth * 0.008f; // us
      period = 2 * period * 0.008f;         // us
      period = 1 / (period * 0.000001f);    // hz

      if (35 <= period && period <= 100) { // save only valid period data (35~100 hz)
        s_channel_data[i].pulse_width_us = pulsewidth;
        s_channel_data[i].last_update_time_ms = time_get_millis();
      }

      pio_interrupt_clear(PIO_CHANNEL, i);
    }
  }
}

static void s_sm_init(PIO pio_hd, uint sm_ch, uint program_offset, uint pin) {
  pio_gpio_init(pio_hd, pin);
  pio_sm_config sm_cfg = pwm_capture_program_get_default_config(program_offset);
  sm_config_set_jmp_pin(&sm_cfg, pin);
  sm_config_set_in_pins(&sm_cfg, pin);
  sm_config_set_in_shift(&sm_cfg, false, false, 0);

  pio_sm_init(pio_hd, sm_ch, program_offset, &sm_cfg);

  pio_set_irq0_source_enabled(pio_hd, (enum pio_interrupt_source)((uint)pis_interrupt0 + sm_ch), true);

  pio_sm_set_enabled(pio_hd, sm_ch, true);
}

bool rc4_init(void) {
  int pio_irq = pio_get_irq_num(PIO_CHANNEL, 0);
  irq_set_exclusive_handler(pio_irq, s_pio_irq_func);
  irq_set_enabled(pio_irq, true);

  uint program_offset = pio_add_program(PIO_CHANNEL, &pwm_capture_program);

  s_sm_init(PIO_CHANNEL, 0, program_offset, HWCONF_PIN_RC_CH1);
  s_sm_init(PIO_CHANNEL, 1, program_offset, HWCONF_PIN_RC_CH2);
  s_sm_init(PIO_CHANNEL, 2, program_offset, HWCONF_PIN_RC_CH3);
  s_sm_init(PIO_CHANNEL, 3, program_offset, HWCONF_PIN_RC_CH4);

  time_block_ms(500); // wait for sm to get first data

  return true;
}

uint32_t rc4_get_ch1_pulsewidth(void) {
  return time_get_millis() - s_channel_data[0].last_update_time_ms < TIMEOUT_MS
             ? s_channel_data[0].pulse_width_us
             : 0;
}

uint32_t rc4_get_ch2_pulsewidth(void) {
  return time_get_millis() - s_channel_data[1].last_update_time_ms < TIMEOUT_MS
             ? s_channel_data[1].pulse_width_us
             : 0;
}

uint32_t rc4_get_ch3_pulsewidth(void) {
  return time_get_millis() - s_channel_data[2].last_update_time_ms < TIMEOUT_MS
             ? s_channel_data[2].pulse_width_us
             : 0;
}

uint32_t rc4_get_ch4_pulsewidth(void) {
  return time_get_millis() - s_channel_data[3].last_update_time_ms < TIMEOUT_MS
             ? s_channel_data[3].pulse_width_us
             : 0;
}

float rc4_get_throttle_percentage(void) {
  uint32_t pulse = rc4_get_ch2_pulsewidth();
  if (pulse == 0) {
    return 0.0f;
  }

  if (HWCONF_RC_THROTTLE_MIDDLE - HWCONF_RC_THROTTLE_PULSE_MARGIN <= pulse &&
      pulse <= HWCONF_RC_THROTTLE_MIDDLE + HWCONF_RC_THROTTLE_PULSE_MARGIN) {
    return 0.0f;
  }

  if (pulse < HWCONF_RC_THROTTLE_MIN_PULSE) {
    pulse = HWCONF_RC_THROTTLE_MIN_PULSE;
  } else if (pulse > HWCONF_RC_THROTTLE_MAX_PULSE) {
    pulse = HWCONF_RC_THROTTLE_MAX_PULSE;
  }

  int32_t min = HWCONF_RC_THROTTLE_MIN_PULSE;
  int32_t max = HWCONF_RC_THROTTLE_MAX_PULSE;
  int32_t center = HWCONF_RC_THROTTLE_MIDDLE;

  float percentage = 0.0f;
  if (pulse >= center) {
    center += HWCONF_RC_THROTTLE_PULSE_MARGIN;
    percentage = -1.0f * (float)(pulse - center) / (max - center) * 100.0f;
  } else {
    center -= HWCONF_RC_THROTTLE_PULSE_MARGIN;
    percentage = (float)(center - pulse) / (center - min) * 100.0f;
  }

  return percentage;
}

float rc4_get_key_degree(void) {
  uint32_t pulse = rc4_get_ch1_pulsewidth();
  if (pulse == 0) {
    return 90.0f;
  }

  if (HWCONF_RC_KEY_MIDDLE - HWCONF_RC_KEY_PULSE_MARGIN <= pulse &&
      pulse <= HWCONF_RC_KEY_MIDDLE + HWCONF_RC_KEY_PULSE_MARGIN) {
    return 90.0f;
  }

  if (pulse < HWCONF_RC_KEY_MIN_PULSE) {
    pulse = HWCONF_RC_KEY_MIN_PULSE;
  } else if (pulse > HWCONF_RC_KEY_MAX_PULSE) {
    pulse = HWCONF_RC_KEY_MAX_PULSE;
  }

  int32_t min = HWCONF_RC_KEY_MIN_PULSE;
  int32_t max = HWCONF_RC_KEY_MAX_PULSE;
  int32_t center = HWCONF_RC_KEY_MIDDLE;

  float percentage = 90.0f;
  if (pulse >= center) {
    center += HWCONF_RC_KEY_PULSE_MARGIN;
    percentage += (float)(pulse - center) / (max - center) * 90.0f;
  } else {
    center -= HWCONF_RC_KEY_PULSE_MARGIN;
    percentage += -1.0f * (float)(center - pulse) / (center - min) * 90.0f;
  }

  return percentage;
}

rc4_slideswitch_data_t rc4_get_slideswitch(void) {
  uint32_t pulse = rc4_get_ch3_pulsewidth();
  if (pulse == 0) {
    return RC4_SLIDESWITCH_ERR;
  }

  if (HWCONF_RC_CH3_MIN_PULSE - 20 <= pulse && pulse <= HWCONF_RC_CH3_MIN_PULSE + 20) {
    return RC4_SLIDESWITCH_BACKWARD;
  } else if (HWCONF_RC_CH3_MAX_PULSE - 20 <= pulse && pulse <= HWCONF_RC_CH3_MAX_PULSE + 20) {
    return RC4_SLIDESWITCH_FORWARD;
  }

  return RC4_SLIDESWITCH_MIDDLE;
}

bool rc4_get_switch(void) {
  uint32_t pulse = rc4_get_ch4_pulsewidth();
  if (pulse == 0) {
    return false;
  }

  if (HWCONF_RC_CH4_CLICKED_PULSE - 20 <= pulse && pulse <= HWCONF_RC_CH4_CLICKED_PULSE + 20) {
    return true;
  }

  return false;
}
