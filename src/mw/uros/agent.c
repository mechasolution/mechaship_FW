#include <rmw_microros/rmw_microros.h>

#include "support.h"

#include "hwconf.h"

#include "driver/log/log.h"

#include "portable/cdc0_transports.h"

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
      cdc0_transport_open,
      cdc0_transport_close,
      cdc0_transport_write,
      cdc0_transport_read);

  return true;
}

static bool s_ping_agent(void) {
  return RMW_RET_OK == rmw_uros_ping_agent(100, 1);
}

static agent_state_t s_agent_state = AGENT_WAIT;
static void s_check_and_action(void) {
  switch (s_agent_state) {
  case AGENT_WAIT:
    if (s_ping_agent()) {
      s_agent_state = AGENT_AVAILABLE;
    } else {
      ;
    }
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
    if (!s_ping_agent()) {
      s_agent_state = AGENT_DISCONNECTED;
      log_debug(TAG, "AGENT_DISCONNECTED");
      break;
    }

    entity_spin();
    break;
  case AGENT_DISCONNECTED:
    entity_destroy();
    s_agent_state = AGENT_WAIT;
    log_debug(TAG, "AGENT_WAIT");
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
