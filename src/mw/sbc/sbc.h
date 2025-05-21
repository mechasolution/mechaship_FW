#ifndef DC5B1F0F_4C2C_4240_8102_67C6719A214A_H_
#define DC5B1F0F_4C2C_4240_8102_67C6719A214A_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  MW_SBC_CONNECTION_NONE,
  MW_SBC_CONNECTION_USB,
  MW_SBC_CONNECTION_CDC,
} mw_sbc_connection_status_t;
typedef void (*mw_sbc_ipv4_change_callback)(uint32_t ipv4);
typedef void (*mw_sbc_connection_change_callback)(mw_sbc_connection_status_t status);

bool mw_sbc_init(void);

bool mw_sbc_report_battery_info(float voltage, float percentage);
bool mw_sbc_report_power_off(void);

void mw_sbc_set_ipv4_change_callback(mw_sbc_ipv4_change_callback callback);
void mw_sbc_set_connection_change_callback(mw_sbc_connection_change_callback callback);

#endif /* DC5B1F0F_4C2C_4240_8102_67C6719A214A_H_ */
