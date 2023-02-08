#include <stdint.h>

#include <std_srvs/srv/trigger.h>

#include "bsp/gy87/include/gy87.h"
#include "bsp/neopixel/include/neopixel.h"
#include "include/gy87_service_callback.h"

extern bsp_neopixel_handler_t neopixel_h;

void gy98_service_offset_calibration_callback(const void *req_p_, void *res_p_) {
  (void *)req_p_;
  // std_srvs__srv__Trigger_Request *req_p = (std_srvs__srv__Trigger_Request *)req_p_;
  std_srvs__srv__Trigger_Response *res_p = (std_srvs__srv__Trigger_Response *)res_p_;

  uint8_t r, g, b, w;
  bsp_neopixel_get_color(&neopixel_h, &r, &g, &b, &w);
  bsp_neopixel_set(&neopixel_h, 255, 100, 0, 0);
  bool ret = bsp_gy87_set_offset_auto();
  bsp_neopixel_set(&neopixel_h, r, g, b, w);
  res_p->success = ret;
}
