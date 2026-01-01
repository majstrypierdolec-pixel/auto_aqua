#include "display.h"

extern LiquidCrystal_I2C lcd(SCREEN_LOCATION, SCREEN_WIDTH, SCREEN_HEIGHT);
uint32_t dimTimer = 0u;
