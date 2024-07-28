#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include "nrf_gpio.h"

typedef struct {
    uint16_t targetTemp;
    uint16_t settleTime;
} TempProfile;

#define PROFILE_COUNT 7

// GPIO

#define GPIO_NUM_11 11
#define GPIO_NUM_12 12
#define GPIO_NUM_13 13
#define GPIO_NUM_14 14
#define GPIO_NUM_15 15
#define GPIO_NUM_16 16
#define GPIO_NUM_17 17

#define BUTTON_GPIO 20

// SPI, assuming a pca10040 board, so the SPI_MOSI_PIN, SPI_SCK_PIN and SPI_MOSI_PIN are already defined in the sdk_config.h file
#define SPI_INSTANCE  0  // SPI instance index
#define SPI_CS_PIN    18 
#define ALERT_PIN     20 


//inmterrupt
#define TIMEOUT_INTERRUPT 30     // 30ms
#define ESP_INTR_FLAG_DEFAULT 0  // default settings

//Profile settings
#define DEFAULT_PROFILE 3
#define MAX_PROFILES 6

//LCD
extern int seven_gpios_map_pin[7];
extern int letter_patterns[3][7];
extern int number_patterns[7][7];

//Temperature profiles
extern TempProfile profiles[PROFILE_COUNT];

#endif