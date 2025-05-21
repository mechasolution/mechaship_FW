#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>

#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/float32.h>

#include "support.h"

#include "driver/log/log.h"

#define TAG "mw/uros/publisher"

static rcl_publisher_t s_battery_voltage_publisher;
static std_msgs__msg__Float32 s_battery_voltage_msg;

static rcl_publisher_t s_emo_status_publisher;
static std_msgs__msg__Bool s_emo_status_msg;

bool publisher_create(rcl_node_t *node) {
  RCCHECK_GOTO(
      TAG,
      rclc_publisher_init_default(
          &s_battery_voltage_publisher,
          node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
          "sensor/battery/voltage"),
      publisher_create_failed);

  RCCHECK_GOTO(
      TAG,
      rclc_publisher_init_default(
          &s_emo_status_publisher,
          node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
          "sensor/emo/status"),
      publisher_create_failed);

  return true;

publisher_create_failed:
  return false;
}

bool publisher_destroy(rcl_node_t *node) {
  RCCHECK_GOTO(
      TAG,
      rcl_publisher_fini(
          &s_battery_voltage_publisher,
          node),
      publisher_destroy_failed);

  RCCHECK_GOTO(
      TAG,
      rcl_publisher_fini(
          &s_emo_status_publisher,
          node),
      publisher_destroy_failed);

  return true;

publisher_destroy_failed:
  return false;
}

bool publisher_publish(uros_pub_data_flag_t data_flag, uros_pub_data_t *data) {
  if (data == NULL) {
    goto publisher_publish_failed;
  }

  switch (data_flag) {
  case UROS_PUB_BATTERY_VOLTAGE:
    s_battery_voltage_msg.data = ((uros_pub_data_t *)data)->battery_voltage.value;
    RCCHECK_GOTO(
        TAG,
        rcl_publish(
            &s_battery_voltage_publisher,
            &s_battery_voltage_msg,
            NULL),
        publisher_publish_failed);
    break;

  case UROS_PUB_EMO_STATUS:
    s_emo_status_msg.data = ((uros_pub_data_t *)data)->emo_status.value;
    RCCHECK_GOTO(
        TAG,
        rcl_publish(
            &s_emo_status_publisher,
            &s_emo_status_msg,
            NULL),
        publisher_publish_failed);
    break;

  case UROS_PUB_NONE:
  case UROS_PUB_MAX:
  default:
    goto publisher_publish_failed;
    // break;
  }

  return true;

publisher_publish_failed:
  return false;
}