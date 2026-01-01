#ifndef DISPLAY_H
#define DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include <stdint.h>

#define SCREEN_LOCATION 0x27
#define SCREEN_WIDTH 16
#define SCREEN_HEIGHT 2

extern LiquidCrystal_I2C lcd;
extern uint32_t dimTimer;

#endif
