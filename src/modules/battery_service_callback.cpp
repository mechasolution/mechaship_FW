#include <stdbool.h>

#include <mechaship_interfaces/srv/battery.h>

#include "bsp/battery/include/battery.h"
#include "include/battery_service_callback.h"

void battery_service_get_data_callback(const void *req_p_, void *res_p_) {
  (void *)req_p_;
  mechaship_interfaces__srv__Battery_Response *res_p = (mechaship_interfaces__srv__Battery_Response *)res_p_;

  double voltage, percentage;
  bool ret = bsp_battery_get(&voltage, &percentage);
  if (ret == false) {
    res_p->status = false;
    res_p->volt = -1;
    res_p->percentage = -1;
    return;
  }
  res_p->status = true;
  res_p->volt = voltage;
  res_p->percentage = percentage;
}