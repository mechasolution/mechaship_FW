#ifndef B4E37264_E97A_47B1_BC6A_601D1291C85A_H_
#define B4E37264_E97A_47B1_BC6A_601D1291C85A_H_

#include <stdbool.h>
#include <stdint.h>

bool battery_init(void);

float battery_get_voltage(void);
float battery_get_percentage(void);

#endif /* B4E37264_E97A_47B1_BC6A_601D1291C85A_H_ */
