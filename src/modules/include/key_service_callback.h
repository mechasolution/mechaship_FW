#ifndef __MODULES_KEY_SERVICE_CALLBACK_H__
#define __MODULES_KEY_SERVICE_CALLBACK_H__

#include "bsp/key/include/key.h"

extern bsp_key_handle_t key_h;

void key_service_set_callback(const void *req_p_, void *res_p_);

#endif