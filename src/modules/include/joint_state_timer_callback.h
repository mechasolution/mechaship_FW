#ifndef __MODULES_JOINT_STATE_TIMER_CALLBACK_H__
#define __MODULES_JOINT_STATE_TIMER_CALLBACK_H__

#include <rcl/rcl.h>

#include "bsp/key/include/key.h"

void joint_state_timer_callback_init(rcl_publisher_t *joint_state_publisher_key_h_p_, bsp_key_handle_t *key_h_p_);
void joint_state_timer_send_callback(rcl_timer_t *timer, int64_t last_call_time);

#endif