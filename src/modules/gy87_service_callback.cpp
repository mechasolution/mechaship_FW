#include <stdint.h>

#include <std_srvs/srv/trigger.h>

#include "bsp/gy87/include/gy87.h"
#include "include/gy87_service_callback.h"

void gy98_service_offset_calibration_callback(const void *req_p_, void *res_p_) {
  (void *)req_p_;
  // std_srvs__srv__Trigger_Request *req_p = (std_srvs__srv__Trigger_Request *)req_p_;
  std_srvs__srv__Trigger_Response *res_p = (std_srvs__srv__Trigger_Response *)res_p_;

  bool ret = bsp_gy87_set_offset_auto();
  res_p->success = ret;
}
