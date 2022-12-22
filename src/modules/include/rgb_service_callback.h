#ifndef __MODULES_RGB_SERVICE_CALLBACK_H__
#define __MODULES_RGB_SERVICE_CALLBACK_H__

#include "bsp/neopixel/include/neopixel.h"

extern bsp_neopixel_handler_t neopixel_h;
void rgb_service_set_callback(const void *req_p_, void *res_p_);

#endif