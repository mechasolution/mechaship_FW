#include <string.h>
#include <timer.h>

#include <Arduino.h>
#include <rclc/rclc.h>

#include <sensor_msgs/msg/imu.h>
#include <sensor_msgs/msg/magnetic_field.h>
#include <std_msgs/msg/header.h>

#include "bsp/gy87/include/gy87.h"
#include "include/gy87_timer_callback.h"

static rcl_publisher_t *s_gy87_publisher_imu_h_p;
static rcl_publisher_t *s_gy87_publisher_mag_h_p;

static sensor_msgs__msg__Imu s_imu_msg;
static sensor_msgs__msg__MagneticField s_mag_msg;

static timespec s_tv;

static bsp_gy87_rtn_data_t s_gy87_data;

static int clock_gettime(clockid_t unused, struct timespec *tp) {
  uint64_t m = time_us_64();
  tp->tv_sec = m / 1000000;
  tp->tv_nsec = (m % 1000000) * 1000;
  return 0;
}

void gy87_timer_callback_init(rcl_publisher_t *gy87_publisher_imu_h_p_, rcl_publisher_t *gy87_publisher_mag_h_p_) {
  s_gy87_publisher_imu_h_p = gy87_publisher_imu_h_p_;
  s_gy87_publisher_mag_h_p = gy87_publisher_mag_h_p_;

  s_mag_msg.header.frame_id.data = s_imu_msg.header.frame_id.data = (char *)"/imu_frame";
  s_mag_msg.header.frame_id.size = s_imu_msg.header.frame_id.size = (strlen("/imu_frame") + 1) * sizeof(char);
}

void gy87_timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
  RCL_UNUSED(last_call_time);

  if (bsp_gy87_get_data(&s_gy87_data) == false) {
    return;
  }

  clock_gettime(CLOCK_REALTIME, &s_tv);
  s_mag_msg.header.stamp.nanosec = s_imu_msg.header.stamp.nanosec = s_tv.tv_nsec;
  s_mag_msg.header.stamp.sec = s_imu_msg.header.stamp.sec = s_tv.tv_sec;

  s_imu_msg.angular_velocity.x = s_gy87_data.gyro.x;
  s_imu_msg.angular_velocity.y = s_gy87_data.gyro.y;
  s_imu_msg.angular_velocity.z = s_gy87_data.gyro.z;

  s_imu_msg.linear_acceleration.x = s_gy87_data.accel.x;
  s_imu_msg.linear_acceleration.y = s_gy87_data.accel.y;
  s_imu_msg.linear_acceleration.z = s_gy87_data.accel.z;

  s_mag_msg.magnetic_field.x = s_gy87_data.mag.x;
  s_mag_msg.magnetic_field.y = s_gy87_data.mag.y;
  s_mag_msg.magnetic_field.z = s_gy87_data.mag.z;

  rcl_publish(s_gy87_publisher_imu_h_p, &s_imu_msg, NULL);
  rcl_publish(s_gy87_publisher_mag_h_p, &s_mag_msg, NULL);
}
