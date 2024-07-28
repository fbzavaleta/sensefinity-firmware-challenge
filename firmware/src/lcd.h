#ifndef LCD_H
#define LCD_H

#include <stdint.h>

extern int seven_gpios_map_pin[7];
extern int letter_patterns[3][7];
extern int number_patterns[7][7];


void lcd_write_letter(char letter);
void lcd_write_number(uint8_t number);
void lcd_clear();
void blink_lcd(uint8_t number, uint32_t ms_delay, char letter);
void init_lcd();

#endif