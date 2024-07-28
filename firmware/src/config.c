#include "config.h"

//Considering a display common cathode
//                                   a            b            c            d            e         f         g
int seven_gpios_map_pin[7] = {GPIO_NUM_11, GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17};

int number_patterns[7][7] = {
    {1, 1, 1, 0, 1, 1, 1}, // 0
    {0, 0, 1, 0, 0, 1, 0}, // 1 
    {1, 0, 1, 1, 1, 0, 1}, // 2 
    {1, 0, 1, 1, 0, 1, 1}, // 3
    {0, 1, 1, 1, 0, 1, 0}, // 4
    {1, 1, 0, 1, 0, 1, 1}, // 5
    {1, 1, 0, 1, 1, 1, 1}, // 6
    };

int letter_patterns[3][7] = {
    {0, 1, 1, 1, 1, 1, 0}, // H
    {0, 1, 0, 0, 1, 0, 1}, // L
    {1, 1, 0, 1, 1, 0, 1}  // E
};

//Temperature profiles
TempProfile profiles[PROFILE_COUNT] = {
    {180, 10},
    {115, 10},
    {238, 30},
    {90,   5},
    {150, 10},
    {75,   5}
};
