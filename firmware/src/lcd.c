#include "lcd.h"
#include "nrf_gpio.h"
#include "FreeRTOS.h"
#include "task.h"

void lcd_write_letter(char letter)
{
    int index = -1;

    // Map letter to index in the letter_patterns array
    if (letter == 'H') {
        index = 0;
    } else if (letter == 'L') {
        index = 1;
    } else if (letter == 'E') {
        index = 2;
    }

    // If index is valid, set the GPIO pins
    if (index != -1) {
        for (int i = 0; i < 7; i++) {
            nrf_gpio_pin_write(seven_gpios_map_pin[i], letter_patterns[index][i]);
        }
    }
}

void lcd_write_number(uint8_t number) {
    if (number > 6) return;
    
    for (int i = 0; i < 7; i++) {
        nrf_gpio_pin_write(seven_gpios_map_pin[i], number_patterns[number][i]);
    }
}

void lcd_clear() {
    for (int i = 0; i < 7; i++) {
        nrf_gpio_pin_write(seven_gpios_map_pin[i], 0);
    }
}

void blink_lcd(uint8_t number, uint32_t ms_delay, char letter) {
    TickType_t delay = pdMS_TO_TICKS(ms_delay);

    if (number >= 0 && number <= 6) {
        lcd_write_number(number);
        vTaskDelay(delay);
    } 
    else if (letter != '\0') {
        lcd_write_letter(letter);
        vTaskDelay(delay);
    }
    else {
        return;
    }
    lcd_clear();
    vTaskDelay(delay);
}

void init_lcd(){
    for (int i = 0; i < 7; i++) 
    {
        nrf_gpio_cfg_output(seven_gpios_map_pin[i]);
    }
}