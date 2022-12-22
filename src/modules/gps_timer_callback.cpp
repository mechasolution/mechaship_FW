#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <sensor_msgs/msg/nav_sat_fix.h>
#include <sensor_msgs/msg/nav_sat_status.h>

#include "bsp/gps/include/gps.h"
#include "include/gps_timer_callback.h"

static rcl_publisher_t *s_gps_publisher_h_p;

static sensor_msgs__msg__NavSatFix s_gps_msg;

static timespec s_tv;

static bsp_gps_rtn_data_t s_gps_data;

void gps_timer_callback_init(rcl_publisher_t *gps_publisher_h_p_) {
  s_gps_publisher_h_p = gps_publisher_h_p_;

  s_gps_msg.header.frame_id.data = (char *)"/gps_frame";
  s_gps_msg.header.frame_id.size = (strlen("/gps_frame" + 1)) * sizeof(char);

  s_gps_msg.status.service = sensor_msgs__msg__NavSatStatus__SERVICE_GPS;
}

void gps_timer_get_callback(rcl_timer_t *timer, int64_t last_call_time) {
  RCL_UNUSED(last_call_time);

  bsp_gps_loop();
}

void gps_timer_send_callback(rcl_timer_t *timer, int64_t last_call_time) {
  RCL_UNUSED(last_call_time);

  if (bsp_gps_get_data(&s_gps_data) == false) {
    return;
  }

  clock_gettime(CLOCK_REALTIME, &s_tv);
  s_gps_msg.header.stamp.nanosec = s_tv.tv_nsec;
  s_gps_msg.header.stamp.sec = s_tv.tv_sec;

  s_gps_msg.status.status = (s_gps_data.fix == true) ? sensor_msgs__msg__NavSatStatus__STATUS_FIX : sensor_msgs__msg__NavSatStatus__STATUS_NO_FIX;
  s_gps_msg.latitude = s_gps_data.latitude;
  s_gps_msg.longitude = s_gps_data.longitude;
  s_gps_msg.altitude = s_gps_data.altitude;

  rcl_publish(s_gps_publisher_h_p, &s_gps_msg, NULL);
}