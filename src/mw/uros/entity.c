#include <rcl/error_handling.h>
#include <rcl/rcl.h>

#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rclc_parameter/rclc_parameter.h>

#include <rmw_microros/rmw_microros.h>

#include "support.h"

#include "driver/log/log.h"
#include "driver/switch8/switch8.h"

#define TAG "mw/microros/entity"

static rclc_support_t s_support;
static rcl_node_t s_node;
static rclc_executor_t s_executor;
static rcl_allocator_t s_allocator;
static rcl_init_options_t s_init_options;

static size_t s_domain_id = 0;

bool entity_create(void) {
  s_allocator = rcl_get_default_allocator();
  s_init_options = rcl_get_zero_initialized_init_options();

  RCCHECK_GOTO(TAG, rcl_init_options_init(&s_init_options, s_allocator), entity_create_failed);

  log_debug(TAG, "micro-ROS Domain ID: %d", s_domain_id);
  RCCHECK_GOTO(TAG, rcl_init_options_set_domain_id(&s_init_options, s_domain_id), entity_create_failed);

  RCCHECK_GOTO(TAG, rclc_support_init_with_options(&s_support, 0, NULL, &s_init_options, &s_allocator), entity_create_failed);
  RCCHECK_GOTO(TAG, rclc_node_init_default(&s_node, "mcu_node", "/", &s_support), entity_create_failed);

  // topic
  publisher_create(&s_node);
  subscriber_create(&s_node);

  // service
  service_create(&s_node);

  // action
  // action_create(&s_node, &s_support);

  s_executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK_GOTO(TAG, rclc_executor_init(&s_executor, &s_support.context, 10, &s_allocator), entity_create_failed);

  subscriber_add_executor(&s_executor);
  service_add_executor(&s_executor);
  // action_add_executor(&s_executor);

  log_debug(TAG, "micro-ROS Init Finished");

  return true;

entity_create_failed:
  log_warning(TAG, "micro-ROS Init Failed");

  return false;
}

bool entity_destroy(void) {
  rmw_context_t *rmw_context = rcl_context_get_rmw_context(&s_support.context);
  RCCHECK_GOTO(TAG, rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0), entity_destroy_failed);

  RCCHECK_GOTO(TAG, rclc_executor_fini(&s_executor), entity_destroy_failed);

  // topic
  publisher_destroy(&s_node);
  subscriber_destroy(&s_node);

  // service
  service_destroy(&s_node);

  // action
  // action_destroy(&s_node);

  RCCHECK_GOTO(TAG, rcl_node_fini(&s_node), entity_destroy_failed);
  RCCHECK_GOTO(TAG, rclc_support_fini(&s_support), entity_destroy_failed);
  RCCHECK_GOTO(TAG, rcl_init_options_fini(&s_init_options), entity_destroy_failed);

  log_debug(TAG, "micro-ROS Deinit Finished");

  return true;

entity_destroy_failed:
  log_warning(TAG, "micro-ROS Deinit Failed");

  return false;
}

void entity_set_domain_id(size_t domain_id) {
  s_domain_id = domain_id;
}

void entity_spin(void) {
  rclc_executor_spin_some(&s_executor, RCL_MS_TO_NS(10));
}
