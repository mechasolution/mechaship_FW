#ifndef E1FAB1B7_42B5_4B32_A584_63D402998FA8_H_
#define E1FAB1B7_42B5_4B32_A584_63D402998FA8_H_

#include <stdbool.h>
#include <stdint.h>

bool lcd_init(void);
bool lcd_reinit_device(void);

void lcd_set_cursor(int line, int position);
void lcd_set_char(char val);
void lcd_set_string(const char *s);
void lcd_next_frame(void);
void lcd_clear(void);

#endif /* E1FAB1B7_42B5_4B32_A584_63D402998FA8_H_ */
