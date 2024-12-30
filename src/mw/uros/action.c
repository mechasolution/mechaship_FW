#include <stdbool.h>
#include <stdint.h>

#include <rcl/rcl.h>

#include <rclc/executor.h>
#include <rclc/rclc.h>

#include <mechaship_interfaces/action/key.h>
#include <mechaship_interfaces/action/thruster.h>

#include "support.h"
#include "uros_types.h"

#include "driver/log/log.h"

#include "hal/time/time.h"

#define TAG "mw/uros/action"

static uros_action_worker_callback_t s_callback_worker = NULL;
static uros_action_goal_callback_t s_callback_goal = NULL;
static uros_action_cancel_callback_t s_callback_cancel = NULL;

static rclc_action_server_t s_key_server;
static uros_action_flag_t s_key_flag = UROS_ACTION_KEY;
static mechaship_interfaces__action__Key_SendGoal_Request s_key_goal;
static mechaship_interfaces__action__Key_FeedbackMessage s_key_feedback;
static mechaship_interfaces__action__Key_GetResult_Response s_key_res = {0};
static rclc_action_goal_handle_t *s_key_goal_hd = NULL;
static bool s_key_worker_en = false;

static rcl_action_server_t s_thruster_server;

void action_worker(bool uart_mutex_get) {
  uros_action_goal_t cb_goal = {0};
  uros_action_feedback_t feedback = {0};
  uros_action_res_t result = {0};
  uros_action_worker_status_t res = UROS_ACTION_WORKER_STATUS_NONE;

  if (s_callback_worker == NULL) {
    return;
  }

  if (s_key_worker_en && s_key_goal_hd != NULL) {
    if (s_key_goal_hd->goal_cancelled == true || s_key_goal_hd->status == GOAL_STATE_CANCELED) {
      if (!uart_mutex_get) {
        return;
      }

      if (rclc_action_send_result(s_key_goal_hd, GOAL_STATE_CANCELED, &s_key_res) != RCL_RET_OK) {
        return;
      }

      s_key_worker_en = false;
    } else {
      cb_goal.key.degrees_per_second = s_key_goal.goal.degrees_per_second;
      cb_goal.key.target_degree = s_key_goal.goal.target_degree;

      res = s_callback_worker(UROS_ACTION_KEY, &cb_goal, &feedback, &result);

      if (!uart_mutex_get) {
        return;
      }

      if (res == UROS_ACTION_WORKER_STATUS_FINISH) {
        s_key_res.result.status = result.key.status;

        if (rclc_action_send_result(s_key_goal_hd, GOAL_STATE_SUCCEEDED, &s_key_res) != RCL_RET_OK) {
          return;
        }

        s_key_worker_en = false;
      } else if (res == UROS_ACTION_WORKER_STATUS_WORKING) {
        s_key_feedback.feedback.current_degree = feedback.key.current_degree;
        rclc_action_publish_feedback(s_key_goal_hd, &s_key_feedback);
      } else {
        s_key_res.result.status = false;
        rclc_action_send_result(s_key_goal_hd, GOAL_STATE_ABORTED, &s_key_res);
        s_key_worker_en = false;
      }
    }
  }
}

static rcl_ret_t s_goal_callback(rclc_action_goal_handle_t *goal_handle, void *action_flag) {
  uros_action_goal_t cb_goal = {0};

  if (s_callback_goal == NULL) {
    return RCL_RET_ACTION_GOAL_REJECTED;
  }

  switch (*(uros_action_flag_t *)action_flag) {
  case UROS_ACTION_KEY:
    cb_goal.key.degrees_per_second = ((mechaship_interfaces__action__Key_SendGoal_Request *)goal_handle->ros_goal_request)->goal.degrees_per_second;
    cb_goal.key.target_degree = ((mechaship_interfaces__action__Key_SendGoal_Request *)goal_handle->ros_goal_request)->goal.target_degree;

    s_key_goal_hd = goal_handle;
    s_key_worker_en = true;

    return s_callback_goal(*(uros_action_flag_t *)action_flag, &cb_goal) == true ? RCL_RET_ACTION_GOAL_ACCEPTED : RCL_RET_ACTION_GOAL_REJECTED;

  default:
    log_warning(TAG, "Unknown flag : %d", *(uros_action_flag_t *)action_flag);
    return RCL_RET_ACTION_GOAL_REJECTED;
  }
}

static bool s_cancel_callback(rclc_action_goal_handle_t *goal_handle, void *action_flag) {
  switch (*(uros_action_flag_t *)action_flag) {
  case UROS_ACTION_KEY:
    s_callback_cancel(*(uros_action_flag_t *)action_flag);
    break;

  default:
    break;
  }

  return true;
}

bool action_create(rcl_node_t *node, rclc_support_t *support) {
  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_action_server_init_default(
          &s_key_server,
          node,
          support,
          ROSIDL_GET_ACTION_TYPE_SUPPORT(mechaship_interfaces, Key),
          "key"));

  // RCCHECK_RETURN_FALSE(
  //     TAG,
  //     rclc_action_server_init_default(
  //         &s_thruster_server,
  //         node,
  //         support,
  //         ROSIDL_GET_ACTION_TYPE_SUPPORT(mechaship_interfaces, Key),
  //         "thruster"));

  return true;
}

bool action_destroy(rcl_node_t *node) {
  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_action_server_fini(
          &s_key_server,
          node));

  // RCCHECK_RETURN_FALSE(
  //     TAG,
  //     rclc_action_server_fini(
  //         &s_thruster_server,
  //         node));

  return true;
}

bool action_add_executor(rclc_executor_t *executor) {
  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_action_server(
          executor,
          &s_key_server,
          1,
          &s_key_goal,
          sizeof(mechaship_interfaces__action__Key_SendGoal_Request),
          s_goal_callback,
          s_cancel_callback,
          (void *)&s_key_flag));

  return true;
}

bool action_set_callback(uros_action_goal_callback_t cb_goal, uros_action_worker_callback_t cb_worker, uros_action_cancel_callback_t cb_cancel) {
  s_callback_worker = cb_worker;
  s_callback_goal = cb_goal;
  s_callback_cancel = cb_cancel;

  return true;
}
