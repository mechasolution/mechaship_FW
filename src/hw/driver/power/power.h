#ifndef D25E1608_C42D_498F_A94F_3A4061B07FE8_H_
#define D25E1608_C42D_498F_A94F_3A4061B07FE8_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool status;
  uint32_t last_change_timestamp;
} power_button_status_t;

typedef void (*power_button_callback_t)(void);

bool power_init(void);

void power_set_button_callback(power_button_callback_t callback);

void power_set_main(bool s);
void power_set_sbc(bool s);
bool power_get_sbc(void);
void power_set_act(bool s);
bool power_get_act(void);

bool power_get_button(void);

#endif /* D25E1608_C42D_498F_A94F_3A4061B07FE8_H_ */
