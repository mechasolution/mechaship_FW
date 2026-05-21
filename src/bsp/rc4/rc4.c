#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>

#include <zephyr/drivers/misc/pio_rpi_pico/pio_rpi_pico.h>

#include "pwm_capture.pio.h"
#include "rc4.h"

#define RC4_NODE DT_NODELABEL(rc_4ch_receiver)
#define RC4_PIO_NODE DT_PHANDLE(RC4_NODE, pio)

#define RC4_CHANNEL0_NODE DT_CHILD(RC4_NODE, channel_0)
#define RC4_CHANNEL1_NODE DT_CHILD(RC4_NODE, channel_1)
#define RC4_CHANNEL2_NODE DT_CHILD(RC4_NODE, channel_2)
#define RC4_CHANNEL3_NODE DT_CHILD(RC4_NODE, channel_3)

#define RC4_CHANNEL_COUNT 4
#define RC4_TIMEOUT_MS 50U
#define RC4_INVALID_CHANNEL UINT8_MAX

BUILD_ASSERT(DT_NODE_HAS_STATUS(RC4_NODE, okay), "rc_4ch_receiver node must be enabled");
BUILD_ASSERT(DT_CHILD_NUM(RC4_NODE) == RC4_CHANNEL_COUNT, "rc_4ch_receiver must define 4 channels");

BUILD_ASSERT(DT_REG_ADDR(RC4_CHANNEL0_NODE) == 0, "channel@0 reg must be 0");
BUILD_ASSERT(DT_REG_ADDR(RC4_CHANNEL1_NODE) == 1, "channel@1 reg must be 1");
BUILD_ASSERT(DT_REG_ADDR(RC4_CHANNEL2_NODE) == 2, "channel@2 reg must be 2");
BUILD_ASSERT(DT_REG_ADDR(RC4_CHANNEL3_NODE) == 3, "channel@3 reg must be 3");

enum rc4_channel_function {
  RC4_CHANNEL_FUNCTION_STEERING = 0,
  RC4_CHANNEL_FUNCTION_THROTTLE,
  RC4_CHANNEL_FUNCTION_AUX1,
  RC4_CHANNEL_FUNCTION_SWITCH,
  RC4_CHANNEL_FUNCTION_COUNT,
};

typedef struct {
  uint8_t pin;
  uint8_t function;
  uint32_t min_pulse_us;
  uint32_t mid_pulse_us;
  uint32_t max_pulse_us;
  uint32_t default_pulse_us;
  uint32_t clicked_pulse_us;
  uint32_t pulse_margin_us;
} rc4_channel_cfg_t;

typedef struct {
  uint8_t sm;
  uint32_t pulse_width_us;
  uint32_t last_update_time_ms;
} rc4_channel_data_t;

#define RC4_CHANNEL_CFG(node_id)                                    \
  {                                                                 \
      .pin = DT_GPIO_PIN(node_id, input_gpios),                     \
      .function = DT_ENUM_IDX(node_id, function),                   \
      .min_pulse_us = DT_PROP_OR(node_id, min_pulse_us, 0),         \
      .mid_pulse_us = DT_PROP_OR(node_id, mid_pulse_us, 0),         \
      .max_pulse_us = DT_PROP_OR(node_id, max_pulse_us, 0),         \
      .default_pulse_us = DT_PROP_OR(node_id, default_pulse_us, 0), \
      .clicked_pulse_us = DT_PROP_OR(node_id, clicked_pulse_us, 0), \
      .pulse_margin_us = DT_PROP(node_id, pulse_margin_us),         \
  }

static const struct device *const s_pio_dev = DEVICE_DT_GET(RC4_PIO_NODE);
static PIO s_pio;

static const rc4_channel_cfg_t s_channel_cfg[RC4_CHANNEL_COUNT] = {
    RC4_CHANNEL_CFG(RC4_CHANNEL0_NODE),
    RC4_CHANNEL_CFG(RC4_CHANNEL1_NODE),
    RC4_CHANNEL_CFG(RC4_CHANNEL2_NODE),
    RC4_CHANNEL_CFG(RC4_CHANNEL3_NODE),
};

static rc4_channel_data_t s_channel_data[RC4_CHANNEL_COUNT];
static uint8_t s_function_to_channel[RC4_CHANNEL_FUNCTION_COUNT] = {
    [0 ... RC4_CHANNEL_FUNCTION_COUNT - 1] = RC4_INVALID_CHANNEL,
};

static uint32_t s_ticks_to_us(uint32_t ticks) {
  return (uint32_t)(2.0f * ticks * 0.008f);
}

static uint32_t s_get_channel_pulsewidth(uint8_t channel_index) {
  uint32_t now_ms;

  if (channel_index >= RC4_CHANNEL_COUNT) {
    return 0;
  }

  now_ms = k_uptime_get_32();
  if ((now_ms - s_channel_data[channel_index].last_update_time_ms) >= RC4_TIMEOUT_MS) {
    return 0;
  }

  return s_channel_data[channel_index].pulse_width_us;
}

static uint32_t s_get_function_pulsewidth(enum rc4_channel_function function) {
  uint8_t channel_index;

  if (function >= RC4_CHANNEL_FUNCTION_COUNT) {
    return 0;
  }

  channel_index = s_function_to_channel[function];
  if (channel_index == RC4_INVALID_CHANNEL) {
    return 0;
  }

  return s_get_channel_pulsewidth(channel_index);
}

static void s_pio_irq_func(const void *arg) {
  ARG_UNUSED(arg);

  for (uint8_t i = 0; i < RC4_CHANNEL_COUNT; i++) {
    uint8_t sm = s_channel_data[i].sm;

    if (!pio_interrupt_get(s_pio, sm)) {
      continue;
    }

    uint32_t pulsewidth_ticks = pio_sm_get(s_pio, sm);
    uint32_t low_ticks = pio_sm_get(s_pio, sm);
    uint32_t pulsewidth_us = s_ticks_to_us(pulsewidth_ticks);
    uint32_t period_us = s_ticks_to_us(pulsewidth_ticks + low_ticks);
    float period_hz = 1.0f / (period_us * 0.000001f);

    if (35.0f <= period_hz && period_hz <= 100.0f) {
      s_channel_data[i].pulse_width_us = pulsewidth_us;
      s_channel_data[i].last_update_time_ms = k_uptime_get_32();
    }

    pio_interrupt_clear(s_pio, sm);
  }
}

static int s_sm_init(uint8_t channel_index, uint program_offset) {
  size_t sm;
  int ret;
  pio_sm_config sm_cfg;
  uint8_t pin = s_channel_cfg[channel_index].pin;

  ret = pio_rpi_pico_allocate_sm(s_pio_dev, &sm);
  if (ret < 0) {
    return ret;
  }

  s_channel_data[channel_index].sm = (uint8_t)sm;

  pio_gpio_init(s_pio, pin);

  sm_cfg = pwm_capture_program_get_default_config(program_offset);
  sm_config_set_jmp_pin(&sm_cfg, pin);
  sm_config_set_in_pins(&sm_cfg, pin);
  sm_config_set_in_shift(&sm_cfg, false, false, 0);

  pio_sm_init(s_pio, sm, program_offset, &sm_cfg);

  pio_set_irq0_source_enabled(s_pio, (enum pio_interrupt_source)((uint)pis_interrupt0 + sm), true);
  pio_sm_set_enabled(s_pio, sm, true);

  return 0;
}

bool rc4_init(void) {
  uint program_offset;

  if (!device_is_ready(s_pio_dev)) {
    return false;
  }

  s_pio = pio_rpi_pico_get_pio(s_pio_dev);

  for (uint8_t i = 0; i < RC4_CHANNEL_COUNT; i++) {
    uint8_t function = s_channel_cfg[i].function;

    s_channel_data[i].sm = RC4_INVALID_CHANNEL;
    s_channel_data[i].pulse_width_us = 0;
    s_channel_data[i].last_update_time_ms = 0;

    if (function < RC4_CHANNEL_FUNCTION_COUNT) {
      s_function_to_channel[function] = i;
    }
  }

  if (!pio_can_add_program(s_pio, &pwm_capture_program)) {
    return false;
  }

  IRQ_CONNECT(DT_IRQ_BY_NAME(RC4_PIO_NODE, irq0, irq),
              DT_IRQ_BY_NAME(RC4_PIO_NODE, irq0, priority),
              s_pio_irq_func, NULL, 0);
  irq_enable(DT_IRQ_BY_NAME(RC4_PIO_NODE, irq0, irq));

  program_offset = pio_add_program(s_pio, &pwm_capture_program);

  for (uint8_t i = 0; i < RC4_CHANNEL_COUNT; i++) {
    if (s_sm_init(i, program_offset) < 0) {
      return false;
    }
  }

  return true;
}

uint32_t rc4_get_ch1_pulsewidth(void) {
  return s_get_channel_pulsewidth(0);
}

uint32_t rc4_get_ch2_pulsewidth(void) {
  return s_get_channel_pulsewidth(1);
}

uint32_t rc4_get_ch3_pulsewidth(void) {
  return s_get_channel_pulsewidth(2);
}

uint32_t rc4_get_ch4_pulsewidth(void) {
  return s_get_channel_pulsewidth(3);
}

float rc4_get_throttle_percentage(void) {
  uint32_t pulse = s_get_function_pulsewidth(RC4_CHANNEL_FUNCTION_THROTTLE);
  uint8_t channel_index = s_function_to_channel[RC4_CHANNEL_FUNCTION_THROTTLE];
  const rc4_channel_cfg_t *cfg;
  int32_t center;
  float percentage = 0.0f;

  if (pulse == 0 || channel_index == RC4_INVALID_CHANNEL) {
    return 0.0f;
  }

  cfg = &s_channel_cfg[channel_index];
  if (cfg->mid_pulse_us - cfg->pulse_margin_us <= pulse &&
      pulse <= cfg->mid_pulse_us + cfg->pulse_margin_us) {
    return 0.0f;
  }

  if (pulse < cfg->min_pulse_us) {
    pulse = cfg->min_pulse_us;
  } else if (pulse > cfg->max_pulse_us) {
    pulse = cfg->max_pulse_us;
  }

  center = (int32_t)cfg->mid_pulse_us;
  if (pulse >= cfg->mid_pulse_us) {
    center += (int32_t)cfg->pulse_margin_us;
    percentage = (float)(pulse - center) / (cfg->max_pulse_us - center) * 100.0f;
  } else {
    center -= (int32_t)cfg->pulse_margin_us;
    percentage = -1.0f * (float)(center - pulse) / (center - (int32_t)cfg->min_pulse_us) * 100.0f;
  }

  return percentage;
}

float rc4_get_key_degree(void) {
  uint32_t pulse = s_get_function_pulsewidth(RC4_CHANNEL_FUNCTION_STEERING);
  uint8_t channel_index = s_function_to_channel[RC4_CHANNEL_FUNCTION_STEERING];
  const rc4_channel_cfg_t *cfg;
  int32_t center;
  float degree = 90.0f;

  if (pulse == 0 || channel_index == RC4_INVALID_CHANNEL) {
    return 90.0f;
  }

  cfg = &s_channel_cfg[channel_index];
  if (cfg->mid_pulse_us - cfg->pulse_margin_us <= pulse &&
      pulse <= cfg->mid_pulse_us + cfg->pulse_margin_us) {
    return 90.0f;
  }

  if (pulse < cfg->min_pulse_us) {
    pulse = cfg->min_pulse_us;
  } else if (pulse > cfg->max_pulse_us) {
    pulse = cfg->max_pulse_us;
  }

  center = (int32_t)cfg->mid_pulse_us;
  if (pulse >= cfg->mid_pulse_us) {
    center += (int32_t)cfg->pulse_margin_us;
    degree += (float)(pulse - center) / (cfg->max_pulse_us - center) * 90.0f;
  } else {
    center -= (int32_t)cfg->pulse_margin_us;
    degree += -1.0f * (float)(center - pulse) / (center - (int32_t)cfg->min_pulse_us) * 90.0f;
  }

  return degree;
}

rc4_slideswitch_data_t rc4_get_slideswitch(void) {
  uint32_t pulse = s_get_function_pulsewidth(RC4_CHANNEL_FUNCTION_AUX1);
  uint8_t channel_index = s_function_to_channel[RC4_CHANNEL_FUNCTION_AUX1];
  const rc4_channel_cfg_t *cfg;

  if (pulse == 0 || channel_index == RC4_INVALID_CHANNEL) {
    return RC4_SLIDESWITCH_ERR;
  }

  cfg = &s_channel_cfg[channel_index];
  if (cfg->min_pulse_us - cfg->pulse_margin_us <= pulse &&
      pulse <= cfg->min_pulse_us + cfg->pulse_margin_us) {
    return RC4_SLIDESWITCH_BACKWARD;
  } else if (cfg->max_pulse_us - cfg->pulse_margin_us <= pulse &&
             pulse <= cfg->max_pulse_us + cfg->pulse_margin_us) {
    return RC4_SLIDESWITCH_FORWARD;
  }

  return RC4_SLIDESWITCH_MIDDLE;
}

bool rc4_get_switch(void) {
  uint32_t pulse = s_get_function_pulsewidth(RC4_CHANNEL_FUNCTION_SWITCH);
  uint8_t channel_index = s_function_to_channel[RC4_CHANNEL_FUNCTION_SWITCH];
  const rc4_channel_cfg_t *cfg;

  if (pulse == 0 || channel_index == RC4_INVALID_CHANNEL) {
    return false;
  }

  cfg = &s_channel_cfg[channel_index];
  if (cfg->clicked_pulse_us - cfg->pulse_margin_us <= pulse &&
      pulse <= cfg->clicked_pulse_us + cfg->pulse_margin_us) {
    return true;
  }

  return false;
}
