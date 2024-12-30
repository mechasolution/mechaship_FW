#include <rmw_microros/rmw_microros.h>

#include "support.h"

#include "hwconf.h"

#include "driver/log/log.h"

#include "portable/uart_transports.h"

#define TAG "mw/microros/agent"

typedef enum {
  AGENT_WAIT,
  AGENT_AVAILABLE,
  AGENT_CONNECTED,

  AGENT_DISCONNECTED,
} agent_state_t;

bool agent_init(void) {
  rmw_uros_set_custom_transport(
      true,
      NULL,
      serial_transport_open,
      serial_transport_close,
      serial_transport_write,
      serial_transport_read);

  return true;
}

static agent_state_t s_agent_state = AGENT_WAIT;
static void s_check_and_action(void) {
  switch (s_agent_state) {
  case AGENT_WAIT:
    EXECUTE_EVERY_N_MS(500, s_agent_state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? AGENT_AVAILABLE : AGENT_WAIT);
    break;
  case AGENT_AVAILABLE:
    log_debug(TAG, "AGENT_AVAILABLE");
    s_agent_state = (true == entity_create()) ? AGENT_CONNECTED : AGENT_WAIT;
    if (s_agent_state == AGENT_WAIT) {
      log_debug(TAG, "AGENT_WAIT");
      entity_destroy();
    } else {
      log_debug(TAG, "AGENT_CONNECTED");
    }
    break;
  case AGENT_CONNECTED:
    EXECUTE_EVERY_N_MS(200, s_agent_state = (RMW_RET_OK == rmw_uros_ping_agent(100, 5)) ? AGENT_CONNECTED : AGENT_DISCONNECTED);
    if (s_agent_state == AGENT_CONNECTED) {
      entity_spin();
    } else {
      log_debug(TAG, "AGENT_DISCONNECTED");
    }
    break;
  case AGENT_DISCONNECTED:
    entity_destroy();
    s_agent_state = AGENT_WAIT;
    break;
  default:
    break;
  }
}

void agent_spin(void) {
  s_check_and_action();
}

bool agent_is_connected(void) {
  return s_agent_state == AGENT_CONNECTED;
}
