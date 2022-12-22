#include <stdbool.h>

#include <mechaship_interfaces/srv/throttle_percentage.h>
#include <mechaship_interfaces/srv/throttle_pulse_width.h>

#include "include/throttle_service_callback.h"

void throttle_service_set_percentage_callback(const void *req_p_, void *res_p_) {
  mechaship_interfaces__srv__ThrottlePercentage_Request *req_p = (mechaship_interfaces__srv__ThrottlePercentage_Request *)req_p_;
  mechaship_interfaces__srv__ThrottlePercentage_Response *res_p = (mechaship_interfaces__srv__ThrottlePercentage_Response *)res_p_;

  if (req_p->percentage > 100) {
    req_p->percentage = 100; // 범위 벗어나는 경우 100으로 잘라줌
  }

  bool ret = bsp_throttle_set_by_percentage(&throttle_h, req_p->percentage);
  res_p->status = ret;
}

void throttle_service_set_pulse_width_callback(const void *req_p_, void *res_p_) {
  mechaship_interfaces__srv__ThrottlePulseWidth_Request *req_p = (mechaship_interfaces__srv__ThrottlePulseWidth_Request *)req_p_;
  mechaship_interfaces__srv__ThrottlePulseWidth_Response *res_p = (mechaship_interfaces__srv__ThrottlePulseWidth_Response *)res_p_;

  bool ret = bsp_throttle_set_by_pulse_width(&throttle_h, req_p->pulse_width);
  res_p->status = ret;
}