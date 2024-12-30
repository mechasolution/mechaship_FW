#ifndef DBA438AD_AA32_4AA4_891D_49A8C6A17097_H_
#define DBA438AD_AA32_4AA4_891D_49A8C6A17097_H_

#include <stdbool.h>
#include <stdint.h>

#include "uros_types.h"

bool uros_init(void);
void uros_set_domain_id(uint8_t domain_id);

void uros_spin(void);
void uros_worker_spin(bool uart_mutex_get);

bool uros_is_connected(void);
bool uros_sub_set_callback(uros_sub_callback_t cb);
bool uros_srv_set_callback(uros_srv_callback_t cb);
bool uros_pub(uros_pub_data_flag_t data_flag, uros_pub_data_t *data);
bool uros_action_set_callback(uros_action_goal_callback_t cb_goal, uros_action_worker_callback_t cb_worker, uros_action_cancel_callback_t cb_cancel);

#endif /* DBA438AD_AA32_4AA4_891D_49A8C6A17097_H_ */
