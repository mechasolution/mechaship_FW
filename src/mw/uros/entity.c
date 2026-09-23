#include <rcl/error_handling.h>
#include <rcl/rcl.h>

#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rclc_parameter/rclc_parameter.h>

#include <rmw_microros/rmw_microros.h>
#include <stdatomic.h>

#include "support.h"

#include "driver/log/log.h"
#include "driver/switch8/switch8.h"

#define TAG "mw/microros/entity"

static rclc_support_t s_support;
static rcl_node_t s_node;
static rclc_executor_t s_executor;
static rcl_allocator_t s_allocator;
static rcl_init_options_t s_init_options;
static bool s_options_ready;
static bool s_support_ready;
static bool s_node_ready;
static bool s_executor_ready;

static atomic_size_t s_domain_id = 0;

bool entity_create(void) {
  if (s_options_ready || s_support_ready || s_node_ready || s_executor_ready) {
    if (!entity_destroy()) return false;
  }
  s_allocator = rcl_get_default_allocator();
  s_init_options = rcl_get_zero_initialized_init_options();

  RCCHECK_GOTO(TAG, rcl_init_options_init(&s_init_options, s_allocator), entity_create_failed);
  s_options_ready = true;

  size_t domain_id = atomic_load(&s_domain_id);
  log_debug(TAG, "micro-ROS Domain ID: %d", (int)domain_id);
  RCCHECK_GOTO(TAG, rcl_init_options_set_domain_id(&s_init_options, domain_id), entity_create_failed);

  RCCHECK_GOTO(TAG, rclc_support_init_with_options(&s_support, 0, NULL, &s_init_options, &s_allocator), entity_create_failed);
  s_support_ready = true;
  RCCHECK_GOTO(TAG, rclc_node_init_default(&s_node, "mcu_node", "/", &s_support), entity_create_failed);
  s_node_ready = true;

  // topic
  if (!publisher_create(&s_node)) goto entity_create_failed;
  if (!subscriber_create(&s_node)) goto entity_create_failed;

  // service
  if (!service_create(&s_node)) goto entity_create_failed;

  // action
  // action_create(&s_node, &s_support);

  s_executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK_GOTO(TAG, rclc_executor_init(&s_executor, &s_support.context, 10, &s_allocator), entity_create_failed);
  s_executor_ready = true;

  if (!subscriber_add_executor(&s_executor)) goto entity_create_failed;
  if (!service_add_executor(&s_executor)) goto entity_create_failed;
  // action_add_executor(&s_executor);

  log_debug(TAG, "micro-ROS Init Finished");

  return true;

entity_create_failed:
  log_warning(TAG, "micro-ROS Init Failed");
  entity_destroy();
  return false;
}

bool entity_destroy(void) {
  bool ok = true;
  if (s_support_ready) {
    rmw_context_t *rmw_context = rcl_context_get_rmw_context(&s_support.context);
    if (rmw_context != NULL && rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0) != RCL_RET_OK) {
      log_warning(TAG, "Failed to set entity destroy timeout");
    }
  }
  if (s_executor_ready) {
    if (rclc_executor_fini(&s_executor) == RCL_RET_OK) s_executor_ready = false;
    else ok = false;
  }
  if (s_node_ready && !s_executor_ready) {
    // The module destroy functions also handle partially created entities.
    if (!service_destroy(&s_node)) ok = false;
    if (!subscriber_destroy(&s_node)) ok = false;
    if (!publisher_destroy(&s_node)) ok = false;
    if (ok && rcl_node_fini(&s_node) == RCL_RET_OK) s_node_ready = false;
    else ok = false;
  }
  if (s_support_ready && !s_node_ready) {
    if (rclc_support_fini(&s_support) == RCL_RET_OK) s_support_ready = false;
    else ok = false;
  }
  if (s_options_ready && !s_support_ready) {
    if (rcl_init_options_fini(&s_init_options) == RCL_RET_OK) s_options_ready = false;
    else ok = false;
  }
  if (!ok) log_warning(TAG, "micro-ROS Deinit Failed");
  else log_debug(TAG, "micro-ROS Deinit Finished");
  return ok;
}

void entity_set_domain_id(size_t domain_id) {
  atomic_store(&s_domain_id, domain_id);
}

void entity_spin(void) {
  rclc_executor_spin_some(&s_executor, RCL_MS_TO_NS(10));
}
