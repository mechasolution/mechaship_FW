#ifndef DD7DA1E5_8E6F_4EF7_B9D9_73473E3B29E7_H_
#define DD7DA1E5_8E6F_4EF7_B9D9_73473E3B29E7_H_

#include <stdbool.h>
#include <stdint.h>

bool led_init(void);
void led_test(bool st);

void led_set_s(bool s);
void led_set_mode(bool is_ros_mode);
void led_set_f(bool s);

#endif /* DD7DA1E5_8E6F_4EF7_B9D9_73473E3B29E7_H_ */
