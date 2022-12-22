#ifndef __MODULES_GPS_TIMER_CALLBACK_H__
#define __MODULES_GPS_TIMER_CALLBACK_H__

#include <rcl/rcl.h>

void gps_timer_callback_init(rcl_publisher_t *gps_publisher_h_p_);
void gps_timer_get_callback(rcl_timer_t *timer, int64_t last_call_time);
void gps_timer_send_callback(rcl_timer_t *timer, int64_t last_call_time);

#endif