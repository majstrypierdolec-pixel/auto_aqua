// Declarations for screen functions used by main.ino
#ifndef SCREENS_H
#define SCREENS_H

#include <stdint.h>
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

const char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

const byte colPins[COLS] = { 37, 39, 41, 43 };
const byte rowPins[ROWS] = { 29, 31, 33, 35 };

extern Keypad keypad;
extern bool editFlag;
void splashScreen();
uint8_t langConfigScreen(uint8_t idx, bool editMode);
int32_t tankVolumeScreen(const char* tankVolumeBuf, bool editMode, uint32_t tankVolume);
int16_t velocityScreen(const char* velocityBuf, int pumpIndex, bool editMode, uint32_t velocity);
void mainScreen(const char* mainScreenBuf, const char* NoTaskBuf);
void lcdPrintWithGlyphs(const char* str, uint8_t length);
void showTime(uint64_t currentTime);
uint64_t timeSetupScreen();

inline uint64_t seconds() {
  static uint32_t previousMillis = 0;
  static uint64_t totalMillis = 0;
  uint32_t currentMillis = millis();
  uint32_t delta;
  if (currentMillis >= previousMillis) {
    delta = currentMillis - previousMillis;
  } else {
    delta = (0xFFFFFFFFUL - previousMillis) + currentMillis + 1;
  }
  totalMillis += delta;
  previousMillis = currentMillis;
  return totalMillis / 1000;
}

#endif
