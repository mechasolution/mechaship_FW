#include <stdio.h>

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#include "hal/time/time.h"
#include "hwconf.h"
#include "lcd.h"
#include "reg.h"

#define LCD_ADDR DT_PROP(HWCONF_NODE, lcd_addr)

#define LCD_CHARACTER 1
#define LCD_COMMAND 0

#define MAX_LINES 2
#define MAX_CHARS 16

static const struct device *s_lcd_i2c = DEVICE_DT_GET(DT_PHANDLE(HWCONF_NODE, lcd_i2c));

static char s_lcd_buff[MAX_LINES][MAX_CHARS];
static uint8_t s_ptr_line;
static uint8_t s_ptr_char;

static void s_i2c_write_byte(uint8_t val) {
  (void)i2c_write(s_lcd_i2c, &val, 1, LCD_ADDR);
}

static void s_lcd_toggle_enable(uint8_t val) {
  s_i2c_write_byte(val | LCD_ENABLE_BIT);
  time_block_us(1);
  s_i2c_write_byte(val & (uint8_t)(~LCD_ENABLE_BIT));
  time_block_us(50);
}

static void s_lcd_send_byte(uint8_t val, int mode) {
  uint8_t high = (uint8_t)(mode | (val & 0xF0) | LCD_BACKLIGHT);
  uint8_t low = (uint8_t)(mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT);

  s_i2c_write_byte(high);
  s_lcd_toggle_enable(high);
  s_i2c_write_byte(low);
  s_lcd_toggle_enable(low);
}

static void s_lcd_reset(void) {
  s_lcd_send_byte(0x03, LCD_COMMAND);
  s_lcd_send_byte(0x03, LCD_COMMAND);
  s_lcd_send_byte(0x03, LCD_COMMAND);
  s_lcd_send_byte(0x02, LCD_COMMAND);

  s_lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
  s_lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
  s_lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);
}

bool lcd_init(void) {
  if (!device_is_ready(s_lcd_i2c)) {
    return false;
  }

  s_lcd_reset();

  for (uint8_t i = 0; i < MAX_LINES; i++) {
    for (uint8_t j = 0; j < MAX_CHARS; j++) {
      s_lcd_buff[i][j] = ' ';
    }
  }

  s_ptr_line = 0;
  s_ptr_char = 0;

  return true;
}

bool lcd_reinit_device(void) {
  s_lcd_reset();

  return true;
}

void lcd_test(bool st) {
  if (st) {
    lcd_set_string("0123456789abcdef");
    lcd_set_cursor(1, 0);
    lcd_set_string("ghijklmnopqrstuv");
    lcd_next_frame();
  } else {
    lcd_clear();
    lcd_next_frame();
  }
}

static void s_lcd_set_cursor(int line, int position) {
  if (line >= MAX_LINES - 1) {
    line = MAX_LINES - 1;
  }
  if (position >= MAX_CHARS - 1) {
    position = MAX_CHARS - 1;
  }

  int val = (line == 0) ? 0x80 + position : 0xC0 + position;
  s_lcd_send_byte((uint8_t)val, LCD_COMMAND);
}

void lcd_next_frame(void) {
  s_lcd_set_cursor(0, 0);

  for (uint8_t i = 0; i < MAX_LINES; i++) {
    for (uint8_t j = 0; j < MAX_CHARS; j++) {
      s_lcd_send_byte((uint8_t)s_lcd_buff[i][j], LCD_CHARACTER);
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

  s_ptr_line = (uint8_t)line;
  s_ptr_char = (uint8_t)position;
}

void lcd_set_char(char val) {
  if (s_ptr_line < MAX_LINES && s_ptr_char < MAX_CHARS) {
    s_lcd_buff[s_ptr_line][s_ptr_char] = val;
  }

  s_ptr_char = (s_ptr_char >= MAX_CHARS - 1) ? (MAX_CHARS - 1) : (uint8_t)(s_ptr_char + 1);
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
