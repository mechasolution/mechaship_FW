#ifndef __MODULES_GY87_TIMER_CALLBACK_H__
#define __MODULES_GY87_TIMER_CALLBACK_H__

#include <rcl/rcl.h>

void gy87_timer_callback_init(rcl_publisher_t *gy87_publisher_imu_h_p_, rcl_publisher_t *gy87_publisher_mag_h_p_);
void gy87_timer_callback(rcl_timer_t *timer, int64_t last_call_time);

#endif