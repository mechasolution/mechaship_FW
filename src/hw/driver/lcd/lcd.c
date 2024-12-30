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

static uint8_t s_pos = 0;

static void s_i2c_write_byte(uint8_t val) {
  i2c_write_blocking(i2c1, LCD_ADDR, &val, 1, false);
}

static void s_lcd_toggle_enable(uint8_t val) {
  time_block_us(2);
  s_i2c_write_byte(val | LCD_ENABLE_BIT);
  time_block_us(2);
  s_i2c_write_byte(val & ~LCD_ENABLE_BIT);
  time_block_us(2);
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

  return true;
}

void lcd_set_cursor(int line, int position) {
  if (line > MAX_LINES) {
    line = MAX_LINES;
  }
  if (position > MAX_CHARS) {
    position = MAX_CHARS;
  }
  s_pos = position;
  int val = (line == 0) ? 0x80 + position : 0xC0 + position;
  s_lcd_send_byte(val, LCD_COMMAND);
}

void lcd_set_char(char val) {
  s_lcd_send_byte(val, LCD_CHARACTER);
}

void lcd_set_string(const char *s) {
  for (uint8_t i = s_pos; i < MAX_CHARS; i++) {
    if (*s == 0) {
      break;
    }
    lcd_set_char(*s);
    s++;
  }
}

void lcd_clear(void) {
  s_lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
}
