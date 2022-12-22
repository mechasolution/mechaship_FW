#include <stdbool.h>
#include <stdint.h>

#include <mechaship_interfaces/srv/key.h>

#include "include/key_service_callback.h"

void key_service_set_callback(const void *req_p_, void *res_p_) {
  mechaship_interfaces__srv__Key_Request *req_p = (mechaship_interfaces__srv__Key_Request *)req_p_;
  mechaship_interfaces__srv__Key_Response *res_p = (mechaship_interfaces__srv__Key_Response *)res_p_;

  bool ret = bsp_key_set(&key_h, req_p->degree);
  res_p->status = ret;
}