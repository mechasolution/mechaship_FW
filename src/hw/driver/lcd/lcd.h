#ifndef E1FAB1B7_42B5_4B32_A584_63D402998FA8_H_
#define E1FAB1B7_42B5_4B32_A584_63D402998FA8_H_

#include <stdbool.h>
#include <stdint.h>

bool lcd_init(void);
void lcd_set_cursor(int line, int position);
void lcd_set_char(char val);
void lcd_set_string(const char *s);
void lcd_clear(void);

#endif /* E1FAB1B7_42B5_4B32_A584_63D402998FA8_H_ */