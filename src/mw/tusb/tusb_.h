#ifndef A3759C50_5252_4507_9D46_D06144D6C528_H_
#define A3759C50_5252_4507_9D46_D06144D6C528_H_

#include <stdbool.h>

// TODO: use usb prefix & wrap tusb api

typedef enum {
  TUSB_CONNECTION_NONE,
  TUSB_CONNECTION_USB,
  TUSB_CONNECTION_CDC,
} tusb_connection_t;

typedef void (*tusb_cdc0_rx_cb)(void);
typedef void (*tusb_cdc1_rx_cb)(void);
typedef void (*tusb_connection_change_cb)(tusb_connection_t status);

bool mw_tusb_init(void);
void mw_tusb_set_cdc0_callback(tusb_cdc0_rx_cb cb);
void mw_tusb_set_cdc1_callback(tusb_cdc1_rx_cb cb);
void mw_tusb_set_connection_change_callback(tusb_connection_change_cb cb);

#endif /* A3759C50_5252_4507_9D46_D06144D6C528_H_ */
