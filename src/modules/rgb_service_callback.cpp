#include <stdbool.h>

#include <mechaship_interfaces/srv/rgb_color.h>

#include "include/rgb_service_callback.h"

void rgb_service_set_callback(const void *req_p_, void *res_p_) {
  mechaship_interfaces__srv__RGBColor_Request *req_p = (mechaship_interfaces__srv__RGBColor_Request *)req_p_;
  mechaship_interfaces__srv__RGBColor_Response *res_p = (mechaship_interfaces__srv__RGBColor_Response *)res_p_;

  bool ret;
  if (req_p->red == req_p->green && req_p->green == req_p->blue && req_p->blue == req_p->red) {
    ret = bsp_neopixel_set(&neopixel_h, 0, 0, 0, req_p->red);
  } else {
    ret = bsp_neopixel_set(&neopixel_h, req_p->red, req_p->green, req_p->blue, 0);
  }

  res_p->status = ret;
}
