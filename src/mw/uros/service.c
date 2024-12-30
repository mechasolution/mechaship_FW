#include <stdbool.h>
#include <stdint.h>

#include <rcl/rcl.h>

#include <rclc/executor.h>
#include <rclc/rclc.h>

#include <mechaship_interfaces/srv/actuator_disable.h>
#include <mechaship_interfaces/srv/actuator_enable.h>
#include <mechaship_interfaces/srv/tone.h>

#include "support.h"
#include "uros_types.h"

#include "driver/log/log.h"

#define TAG "mw/uros/service"

static uros_srv_callback_t s_callback = NULL;

static rcl_service_t s_actuator_enable_service;
static uros_srv_req_flag_t s_actuator_enable_flag = UROS_SRV_ACTUATOR_ENABLE;
static mechaship_interfaces__srv__ActuatorEnable_Request s_actuator_enable_req;
static mechaship_interfaces__srv__ActuatorEnable_Response s_actuator_enable_res;

static rcl_service_t s_actuator_disable_service;
static uros_srv_req_flag_t s_actuator_disable_flag = UROS_SRV_ACTUATOR_DISABLE;
static mechaship_interfaces__srv__ActuatorEnable_Request s_actuator_disable_req;
static mechaship_interfaces__srv__ActuatorDisable_Response s_actuator_disable_res;

static rcl_service_t s_tone_service;
static uros_srv_req_flag_t s_tone_flag = UROS_SRV_TONE;
static mechaship_interfaces__srv__Tone_Request s_tone_req;
static mechaship_interfaces__srv__Tone_Response s_tone_res;

static void s_service_callback(const void *req, void *res, void *req_flag) {
  uros_srv_req_t cb_req = {0};
  uros_srv_res_t cb_res = {0};

  if (s_callback == NULL) {
    return;
  }

  switch (*(uros_srv_req_flag_t *)req_flag) {
  case UROS_SRV_ACTUATOR_ENABLE:
    cb_req.actuator_enable.key_max_degree = ((mechaship_interfaces__srv__ActuatorEnable_Request *)req)->key_max_degree;
    cb_req.actuator_enable.key_min_degree = ((mechaship_interfaces__srv__ActuatorEnable_Request *)req)->key_min_degree;
    cb_req.actuator_enable.key_pulse_0_degree = ((mechaship_interfaces__srv__ActuatorEnable_Request *)req)->key_pulse_0_degree;
    cb_req.actuator_enable.key_pulse_180_degree = ((mechaship_interfaces__srv__ActuatorEnable_Request *)req)->key_pulse_180_degree;
    cb_req.actuator_enable.thruster_pulse_0_percentage = ((mechaship_interfaces__srv__ActuatorEnable_Request *)req)->thruster_pulse_0_percentage;
    cb_req.actuator_enable.thruster_pulse_100_percentage = ((mechaship_interfaces__srv__ActuatorEnable_Request *)req)->thruster_pulse_100_percentage;

    s_callback(*(uros_srv_req_flag_t *)req_flag, &cb_req, &cb_res);

    ((mechaship_interfaces__srv__ActuatorEnable_Response *)res)->status = cb_res.actuator_enable.status;
    break;

  case UROS_SRV_ACTUATOR_DISABLE:
    s_callback(*(uros_srv_req_flag_t *)req_flag, NULL, &cb_res);

    ((mechaship_interfaces__srv__ActuatorDisable_Response *)res)->status = cb_res.actuator_disable.status;
    break;

  case UROS_SRV_TONE:
    cb_req.tone.hz = ((mechaship_interfaces__srv__Tone_Request *)req)->hz;
    cb_req.tone.duration_ms = ((mechaship_interfaces__srv__Tone_Request *)req)->duration_ms;

    s_callback(*(uros_srv_req_flag_t *)req_flag, &cb_req, &cb_res);

    ((mechaship_interfaces__srv__Tone_Response *)res)->status = cb_res.tone.status;
    break;

  case UROS_SRV_NONE:
  case UROS_SRV_MAX:
  default:
    log_warning(TAG, "Unknown flag : %d", *(uros_srv_req_flag_t *)req_flag);
    break;
  }
}

bool service_create(rcl_node_t *node) {
  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_service_init_default(
          &s_actuator_enable_service,
          node,
          ROSIDL_GET_SRV_TYPE_SUPPORT(mechaship_interfaces, srv, ActuatorEnable),
          "system/actuator/enable"));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_service_init_default(
          &s_actuator_disable_service,
          node,
          ROSIDL_GET_SRV_TYPE_SUPPORT(mechaship_interfaces, srv, ActuatorDisable),
          "system/actuator/disable"));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_service_init_default(
          &s_tone_service,
          node,
          ROSIDL_GET_SRV_TYPE_SUPPORT(mechaship_interfaces, srv, Tone),
          "actuator/tone/addqueue"));

  return true;
}

bool service_destroy(rcl_node_t *node) {
  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_service_fini(
          &s_actuator_enable_service,
          node));

  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_service_fini(
          &s_actuator_disable_service,
          node));

  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_service_fini(
          &s_tone_service,
          node));

  return true;
}

bool service_add_executor(rclc_executor_t *executor) {
  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_service_with_context(
          executor,
          &s_actuator_enable_service,
          &s_actuator_enable_req,
          &s_actuator_enable_res,
          s_service_callback,
          &s_actuator_enable_flag));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_service_with_context(
          executor,
          &s_actuator_disable_service,
          &s_actuator_disable_req,
          &s_actuator_disable_res,
          s_service_callback,
          &s_actuator_disable_flag));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_service_with_context(
          executor,
          &s_tone_service,
          &s_tone_req,
          &s_tone_res,
          s_service_callback,
          &s_tone_flag));

  return true;
}

bool service_set_callback(uros_srv_callback_t cb) {
  s_callback = cb;
  return true;
}
