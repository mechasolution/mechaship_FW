#ifndef __MODULES_THROTTLE_SERVICE_CALLBACK_H__
#define __MODULES_THROTTLE_SERVICE_CALLBACK_H__

#include "bsp/throttle/include/throttle.h"

extern bsp_throttle_handle_t throttle_h;

void throttle_service_set_percentage_callback(const void *req_p_, void *res_p_);
void throttle_service_set_pulse_width_callback(const void *req_p_, void *res_p_);

#endif