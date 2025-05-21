#ifndef C3222403_9C7A_4DC7_98F9_C4A1E48BEAA1_H_
#define C3222403_9C7A_4DC7_98F9_C4A1E48BEAA1_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  RC4_SLIDESWITCH_ERR = 0x00,

  RC4_SLIDESWITCH_MIDDLE,
  RC4_SLIDESWITCH_BACKWARD,
  RC4_SLIDESWITCH_FORWARD,
} rc4_slideswitch_data_t;

typedef void (*rc4_ch3_change_callback_t)(const rc4_slideswitch_data_t status);
typedef void (*rc4_ch4_change_callback_t)(const bool is_pressed);

bool rc4_init(void);

uint32_t rc4_get_ch1_pulsewidth(void);
uint32_t rc4_get_ch2_pulsewidth(void);
uint32_t rc4_get_ch3_pulsewidth(void);
uint32_t rc4_get_ch4_pulsewidth(void);

float rc4_get_throttle_percentage(void);
float rc4_get_key_degree(void);
rc4_slideswitch_data_t rc4_get_slideswitch(void);
bool rc4_get_switch(void);

#endif /* C3222403_9C7A_4DC7_98F9_C4A1E48BEAA1_H_ */
