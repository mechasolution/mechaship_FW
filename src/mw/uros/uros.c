#include <rmw_microros/rmw_microros.h>

#include <rcl/error_handling.h>
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>

#include "support.h"

#include "uros.h"

static bool s_agent_connected = false;

bool uros_init(void) {
  bool ret = true;

  ret &= agent_init();

  return ret;
}

void uros_set_domain_id(uint8_t domain_id) {
  entity_set_domain_id((size_t)domain_id);
}

void uros_spin(void) {
  agent_spin();
}

void uros_worker_spin(bool uart_mutex_get) {
  action_worker(uart_mutex_get);
}

bool uros_is_connected(void) {
  return agent_is_connected();
}

bool uros_sub_set_callback(uros_sub_callback_t cb) {
  return subscriber_set_callback(cb);
}

bool uros_srv_set_callback(uros_srv_callback_t cb) {
  return service_set_callback(cb);
}

bool uros_action_set_callback(uros_action_goal_callback_t cb_goal, uros_action_worker_callback_t cb_worker, uros_action_cancel_callback_t cb_cancel) {
  return action_set_callback(cb_goal, cb_worker, cb_cancel);
}

bool uros_pub(uros_pub_data_flag_t data_flag, uros_pub_data_t *data) {
  return publisher_publish(data_flag, data);
}
