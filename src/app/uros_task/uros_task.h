#ifndef BEE4BF92_C833_4F6E_8E9C_70ED87F16A0C_H_
#define BEE4BF92_C833_4F6E_8E9C_70ED87F16A0C_H_

#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>

bool uros_task_init(void);
bool uros_task_deinit(void);

void uros_task_noti_ip_changed(void);

#endif /* BEE4BF92_C833_4F6E_8E9C_70ED87F16A0C_H_ */
