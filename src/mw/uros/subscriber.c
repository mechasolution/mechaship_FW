#include <stdbool.h>
#include <stdint.h>

#include <rcl/rcl.h>

#include <rclc/executor.h>
#include <rclc/rclc.h>

#include <mechaship_interfaces/msg/rgbw_led_color.h>
#include <mechaship_interfaces/msg/tone_topic.h>
#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/float32.h>
#include <std_msgs/msg/u_int32.h>

#include "support.h"
#include "uros_types.h"

#include "driver/log/log.h"

#define TAG "mw/uros/subscriber"

static uros_sub_callback_t s_callback = NULL;

static rcl_subscription_t s_key_subscription;
static uros_sub_data_flag_t s_key_msg_flag = UROS_SUB_KEY;
static std_msgs__msg__Float32 s_key_msg;

static rcl_subscription_t s_throttle_subscription;
static uros_sub_data_flag_t s_throttle_msg_flag = UROS_SUB_THROTTLE;
static std_msgs__msg__Float32 s_throttle_msg;

static rcl_subscription_t s_led_user_1_subscription;
static uros_sub_data_flag_t s_led_user_1_msg_flag = UROS_SUB_LED_USER_1;
static std_msgs__msg__Bool s_led_user_1_msg;

static rcl_subscription_t s_led_user_2_subscription;
static uros_sub_data_flag_t s_led_user_2_msg_flag = UROS_SUB_LED_USER_2;
static std_msgs__msg__Bool s_led_user_2_msg;

static rcl_subscription_t s_tone_subscription;
static uros_sub_data_flag_t s_tone_msg_flag = UROS_SUB_TONE;
static mechaship_interfaces__msg__ToneTopic s_tone_msg;

static rcl_subscription_t s_rgbw_led_subscription;
static uros_sub_data_flag_t s_rgbw_led_msg_flag = UROS_SUB_RGBW_LED;
static mechaship_interfaces__msg__RgbwLedColor s_rgbw_led_msg;

static rcl_subscription_t s_ip_addr_subscription;
static uros_sub_data_flag_t s_ip_addr_msg_flag = UROS_SUB_IP_ADDR;
static std_msgs__msg__UInt32 s_ip_addr_msg;

static void s_subscriber_callback(const void *data, void *data_flag) {
  uros_sub_data_t cb_data = {0};

  if (s_callback == NULL) {
    return;
  }

  switch (*(uros_sub_data_flag_t *)data_flag) {
  case UROS_SUB_KEY:
    cb_data.key_degree.degree = ((uros_sub_data_t *)data)->key_degree.degree;
    break;

  case UROS_SUB_THROTTLE:
    cb_data.throttle_percentage.percentage = ((uros_sub_data_t *)data)->throttle_percentage.percentage;
    break;

  case UROS_SUB_LED_USER_1:
    cb_data.led_user_1.value = ((uros_sub_data_t *)data)->led_user_1.value;
    break;

  case UROS_SUB_LED_USER_2:
    cb_data.led_user_2.value = ((uros_sub_data_t *)data)->led_user_2.value;
    break;

  case UROS_SUB_TONE:
    cb_data.tone.hz = ((uros_sub_data_t *)data)->tone.hz;
    cb_data.tone.duration_ms = ((uros_sub_data_t *)data)->tone.duration_ms;
    break;

  case UROS_SUB_RGBW_LED:
    cb_data.rgbw_led.red = ((uros_sub_data_t *)data)->rgbw_led.red;
    cb_data.rgbw_led.green = ((uros_sub_data_t *)data)->rgbw_led.green;
    cb_data.rgbw_led.blue = ((uros_sub_data_t *)data)->rgbw_led.blue;
    cb_data.rgbw_led.white = ((uros_sub_data_t *)data)->rgbw_led.white;
    break;

  case UROS_SUB_IP_ADDR:
    cb_data.ip_addr.value = ((uros_sub_data_t *)data)->ip_addr.value;
    break;

  case UROS_SUB_MAX:
  case UROS_SUB_NONE:
  default:
    log_warning(TAG, "Unknown flag : %d", *(uros_sub_data_flag_t *)data_flag);
    break;
  }

  s_callback(*(uros_sub_data_flag_t *)data_flag, &cb_data);
}

bool subscriber_set_callback(uros_sub_callback_t cb) {
  if (cb == NULL) {
    return false;
  }

  s_callback = cb;

  return true;
}

bool subscriber_create(rcl_node_t *node) {
  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_subscription_init_default(
          &s_key_subscription,
          node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
          "actuator/key/degree"));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_subscription_init_default(
          &s_throttle_subscription,
          node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
          "actuator/thruster/percentage"));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_subscription_init_default(
          &s_led_user_1_subscription,
          node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
          "actuator/led/user_1"));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_subscription_init_default(
          &s_led_user_2_subscription,
          node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
          "actuator/led/user_2"));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_subscription_init_default(
          &s_tone_subscription,
          node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(mechaship_interfaces, msg, ToneTopic),
          "actuator/tone/play"));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_subscription_init_default(
          &s_rgbw_led_subscription,
          node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(mechaship_interfaces, msg, RgbwLedColor),
          "actuator/rgbwled/color"));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_subscription_init_default(
          &s_ip_addr_subscription,
          node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, UInt32),
          "system/ip_address_report"));

  return true;
}

bool subscriber_destroy(rcl_node_t *node) {
  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_subscription_fini(
          &s_key_subscription,
          node));

  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_subscription_fini(
          &s_throttle_subscription,
          node));

  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_subscription_fini(
          &s_led_user_1_subscription,
          node));

  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_subscription_fini(
          &s_led_user_2_subscription,
          node));

  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_subscription_fini(
          &s_tone_subscription,
          node));

  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_subscription_fini(
          &s_rgbw_led_subscription,
          node));

  RCCHECK_RETURN_FALSE(
      TAG,
      rcl_subscription_fini(
          &s_ip_addr_subscription,
          node));

  return true;
}

bool subscriber_add_executor(rclc_executor_t *executor) {
  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_subscription_with_context(
          executor,
          &s_key_subscription,
          &s_key_msg,
          s_subscriber_callback,
          &s_key_msg_flag,
          ON_NEW_DATA));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_subscription_with_context(
          executor,
          &s_throttle_subscription,
          &s_throttle_msg,
          s_subscriber_callback,
          &s_throttle_msg_flag,
          ON_NEW_DATA));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_subscription_with_context(
          executor,
          &s_led_user_1_subscription,
          &s_led_user_1_msg,
          s_subscriber_callback,
          &s_led_user_1_msg_flag,
          ON_NEW_DATA));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_subscription_with_context(
          executor,
          &s_led_user_2_subscription,
          &s_led_user_2_msg,
          s_subscriber_callback,
          &s_led_user_2_msg_flag,
          ON_NEW_DATA));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_subscription_with_context(
          executor,
          &s_tone_subscription,
          &s_tone_msg,
          s_subscriber_callback,
          &s_tone_msg_flag,
          ON_NEW_DATA));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_subscription_with_context(
          executor,
          &s_rgbw_led_subscription,
          &s_rgbw_led_msg,
          s_subscriber_callback,
          &s_rgbw_led_msg_flag,
          ON_NEW_DATA));

  RCCHECK_RETURN_FALSE(
      TAG,
      rclc_executor_add_subscription_with_context(
          executor,
          &s_ip_addr_subscription,
          &s_ip_addr_msg,
          s_subscriber_callback,
          &s_ip_addr_msg_flag,
          ON_NEW_DATA));

  return true;
}