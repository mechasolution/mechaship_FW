#include <stdio.h>

#include <hardware/gpio.h>
#include <hardware/i2c.h>

#include "hal/time/time.h"

#include "lcd.h"
#include "reg.h"

#define LCD_ADDR 0x27

#define LCD_CHARACTER 1
#define LCD_COMMAND 0

#define MAX_LINES 2
#define MAX_CHARS 16

static char s_lcd_buff[MAX_LINES][MAX_CHARS] = {0};
static uint8_t s_ptr_line = 0, s_ptr_char = 0;

static void s_i2c_write_byte(uint8_t val) {
  i2c_write_blocking(i2c1, LCD_ADDR, &val, 1, false);
}

static void s_lcd_toggle_enable(uint8_t val) {
  time_block_us(600);
  s_i2c_write_byte(val | LCD_ENABLE_BIT);
  time_block_us(600);
  s_i2c_write_byte(val & ~LCD_ENABLE_BIT);
  time_block_us(600);
}

static void s_lcd_send_byte(uint8_t val, int mode) {
  uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
  uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

  s_i2c_write_byte(high);
  s_lcd_toggle_enable(high);
  s_i2c_write_byte(low);
  s_lcd_toggle_enable(low);
}

bool lcd_init(void) {
  i2c_init(i2c1, 100 * 1000);
  gpio_set_function(HWCONF_PIN_I2C1_SDA, GPIO_FUNC_I2C);
  gpio_set_function(HWCONF_PIN_I2C1_SCL, GPIO_FUNC_I2C);

  s_lcd_send_byte(0x03, LCD_COMMAND);
  s_lcd_send_byte(0x03, LCD_COMMAND);
  s_lcd_send_byte(0x03, LCD_COMMAND);
  s_lcd_send_byte(0x02, LCD_COMMAND);

  s_lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
  s_lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
  s_lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);

  lcd_clear();
  lcd_next_frame();

  return true;
}

void s_lcd_set_cursor(int line, int position) {
  if (line >= MAX_LINES - 1) {
    line = MAX_LINES - 1;
  }
  if (position >= MAX_CHARS - 1) {
    position = MAX_CHARS - 1;
  }

  int val = (line == 0) ? 0x80 + position : 0xC0 + position;
  s_lcd_send_byte(val, LCD_COMMAND);
}

void lcd_next_frame(void) {
  s_lcd_send_byte(0x03, LCD_COMMAND);
  s_lcd_send_byte(0x03, LCD_COMMAND);
  s_lcd_send_byte(0x03, LCD_COMMAND);
  s_lcd_send_byte(0x02, LCD_COMMAND);

  s_lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
  s_lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
  s_lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);

  s_lcd_set_cursor(0, 0);

  for (uint8_t i = 0; i < MAX_LINES; i++) {
    for (uint8_t j = 0; j < MAX_CHARS; j++) {
      s_lcd_send_byte(s_lcd_buff[i][j], LCD_CHARACTER);
    }
    s_lcd_set_cursor(1, 0);
  }
}

void lcd_set_cursor(int line, int position) {
  if (line >= MAX_LINES - 1) {
    line = MAX_LINES - 1;
  }
  if (position >= MAX_CHARS - 1) {
    position = MAX_CHARS - 1;
  }

  s_ptr_line = line;
  s_ptr_char = position;
}

void lcd_set_char(char val) {
  if (s_ptr_line < MAX_LINES && s_ptr_char < MAX_CHARS) {
    s_lcd_buff[s_ptr_line][s_ptr_char] = val;
  }

  s_ptr_char = (s_ptr_char >= MAX_CHARS - 1) ? MAX_CHARS - 1 : s_ptr_char + 1;
}

void lcd_set_string(const char *s) {
  for (uint8_t i = s_ptr_char; i < MAX_CHARS; i++) {
    if (*s == 0) {
      break;
    }

    lcd_set_char(*s);
    s++;
  }
}

void lcd_clear(void) {
  for (uint8_t i = 0; i < MAX_LINES; i++) {
    for (uint8_t j = 0; j < MAX_CHARS; j++) {
      s_lcd_buff[i][j] = ' ';
    }
  }

  s_ptr_line = 0;
  s_ptr_char = 0;
}
