#ifndef C1EE62E1_EA3A_4A85_A6AC_A78A658DF633_H_
#define C1EE62E1_EA3A_4A85_A6AC_A78A658DF633_H_

#include <stdbool.h>
#include <stdint.h>

bool tone_init(void);
void tone_set(uint16_t hz);
inline void tone_reset(void) {
  tone_set(0);
}

#endif /* C1EE62E1_EA3A_4A85_A6AC_A78A658DF633_H_ */
