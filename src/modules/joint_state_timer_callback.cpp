#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <timer.h>

#include <sensor_msgs/msg/joint_state.h>

#include "include/joint_state_timer_callback.h"

static sensor_msgs__msg__JointState s_joint_state_key_msg;
static rcl_publisher_t *s_joint_state_publisher_key_h_p;
static bsp_key_handle_t *s_key_h_p;
static timespec s_tv;
static uint16_t s_key_degree_data;

static int clock_gettime(clockid_t unused, struct timespec *tp) {
  uint64_t m = time_us_64();
  tp->tv_sec = m / 1000000;
  tp->tv_nsec = (m % 1000000) * 1000;
  return 0;
}

void joint_state_timer_callback_init(rcl_publisher_t *joint_state_publisher_key_h_p_, bsp_key_handle_t *key_h_p_) {
  s_joint_state_publisher_key_h_p = joint_state_publisher_key_h_p_;
  s_key_h_p = key_h_p_;

  s_joint_state_key_msg.name.data = (rosidl_runtime_c__String *)malloc(1 * sizeof(rosidl_runtime_c__String));
  s_joint_state_key_msg.name.size = 1;
  s_joint_state_key_msg.name.data[0].data = (char *)"thruster_joint";
  s_joint_state_key_msg.name.data[0].size = strlen("thruster_joint") + 1;

  s_joint_state_key_msg.position.data = (double *)malloc(1 * sizeof(double));
  s_joint_state_key_msg.position.size = 1;
}

void joint_state_timer_send_callback(rcl_timer_t *timer, int64_t last_call_time) {
  if (bsp_key_get_degree(s_key_h_p, &s_key_degree_data) == false) {
    return;
  }

  clock_gettime(CLOCK_REALTIME, &s_tv);
  s_joint_state_key_msg.header.stamp.nanosec = s_tv.tv_nsec;
  s_joint_state_key_msg.header.stamp.sec = s_tv.tv_sec;
  s_joint_state_key_msg.position.data[0] = ((double)s_key_degree_data - 90) * (M_PI / 180.0);

  rcl_publish(s_joint_state_publisher_key_h_p, &s_joint_state_key_msg, NULL);
}
