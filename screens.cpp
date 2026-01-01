#include <Arduino.h>
#include <Keypad.h>
#include "chars.h"
#include "display.h"
#include "language.h"
#include "screens.h"

// ============================================================================
// SCREENS.CPP - Screen implementations
// ============================================================================

// NOTE: readLanguage() needs to be implemented in storage.cpp //why tho.
// Odczyt pojedynczego pola z PROGMEM do bufora docelowego

void readLanguageField(uint8_t idx, uint8_t offset, char* dest, uint8_t len) {
  const void* base = (const void*)&LANGUAGES[idx % LANG_COUNT];
  memcpy_P(dest, (const void*)((uintptr_t)base + offset), len);
  dest[len] = '\0';
}

// Odczyt całej struktury Language tylko jeśli naprawdę potrzebny
Language readLanguage(uint8_t idx) {
  Language result;
  memcpy_P(&result, &LANGUAGES[idx % LANG_COUNT], sizeof(Language));
  return result;
}

Keypad keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
bool editFlag = false;

void splashScreen() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(3, 0);
  lcd.print("AUTO");
  lcd.setCursor(4, 1);
  lcd.print("AQUA");

  uint8_t slots[4] = { 0, 1, 2, 3 };

  for (uint8_t r = 0; r <= 8; r++) {
    lcd.setCursor(11, 0);
    lcd.write(0);
    lcd.setCursor(10, 1);
    lcd.write(1);
    lcd.write(2);
    lcd.write(3);

    animateIcon(slots, r, scratch);
    delay(80);
  }

  delay(1000);
}

uint8_t langConfigScreen(uint8_t idx, bool editMode) {
  lcd.clear();
  loadGlyphSet(idx);
  char langName[LANG_NAME_LEN+1];
  char langPrompt[LANG_PROMPT_LEN+1];
  readLanguageField(idx, offsetof(Language, langName), langName, LANG_NAME_VISIBLE);
  readLanguageField(idx, offsetof(Language, langPrompt), langPrompt, LANG_PROMPT_VISIBLE);
  lcd.setCursor(0, 0);
  lcdPrintWithGlyphs(langName, LANG_NAME_VISIBLE);
  lcd.setCursor(0, 1);
  lcd.print("Num=");
  lcdPrintWithGlyphs(langPrompt, LANG_PROMPT_VISIBLE);
  lcd.print("  #->");

  uint8_t newlang = idx;
  while (true) {
    char key = keypad.getKey();

    if (key) {
      if (key >= '0' && key <= '9') {
        newlang = key - '0';
      } else if (key == 'A') {
        newlang = (newlang + 1) % LANG_COUNT;
      } else if (key == 'B') {
        newlang = (newlang + LANG_COUNT - 1) % LANG_COUNT;
      } else if (key == '#') {
        return newlang;
      } else if (key == '*') {
        return idx;
      }

      if (newlang != idx) {
        idx = newlang;
        loadGlyphSet(idx);
        readLanguageField(idx, offsetof(Language, langName), langName, LANG_NAME_VISIBLE);
        readLanguageField(idx, offsetof(Language, langPrompt), langPrompt, LANG_PROMPT_VISIBLE);
        lcd.setCursor(0, 0);
        lcdPrintWithGlyphs(langName, LANG_NAME_VISIBLE);
        lcd.setCursor(4, 1);
        lcdPrintWithGlyphs(langPrompt, LANG_PROMPT_VISIBLE);
      }
    }

    delay(100);
  }
}


// Formatki LCD do PROGMEM
const char LCD_TANK_FORMAT[] PROGMEM = "<-* _______l #->";
const char LCD_VEL_FORMAT[]  PROGMEM = "<-* ____ml/s #->";

// Wspólna funkcja pomocnicza do edycji liczby na ekranie
int32_t editNumberScreen(const char* label, const char* format, uint8_t entryCol, uint8_t maxDigits, uint32_t value, bool editMode, const char* unit = nullptr) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(label);
  lcd.setCursor(0, 1);
  // Pobierz formatkę z PROGMEM do bufora o minimalnym rozmiarze
  char fmtBuf[20];
  strncpy_P(fmtBuf, format, sizeof(fmtBuf)-1);
  fmtBuf[sizeof(fmtBuf)-1] = '\0';
  lcd.print(fmtBuf);

  unsigned long lastBlink = millis();
  bool showCursor = false;
  bool digitsEntered = (value != 0);
  uint8_t lastDigitPos = entryCol + maxDigits - 1;
  char lastDigitChar = ' ';
  uint8_t curLen = 0;

  auto redrawNumber = [&](uint32_t val) {
    lcd.setCursor(entryCol, 1);
    for (uint8_t i = 0; i < maxDigits; i++) lcd.print('_');
    if (!digitsEntered) {
      curLen = 0;
      lastDigitPos = entryCol + maxDigits - 1;
      lastDigitChar = ' ';
      return;
    }
    char tmp[8]; // max 7 cyfr + null
    uint8_t idx = 0;
    unsigned long v = (unsigned long)val;
    if (v == 0) {
      tmp[idx++] = '0';
    } else {
      char rev[8];
      uint8_t ri = 0;
      while (v > 0 && ri < sizeof(rev)) {
        rev[ri++] = '0' + (v % 10);
        v /= 10;
      }
      for (int8_t i = ri - 1; i >= 0; --i) tmp[idx++] = rev[i];
    }
    tmp[idx] = '\0';
    if (idx > maxDigits) {
      const char* p = tmp + (idx - maxDigits);
      lcd.setCursor(entryCol, 1);
      lcd.print(p);
      curLen = maxDigits;
      lastDigitPos = entryCol + maxDigits - 1;
      lastDigitChar = p[maxDigits - 1];
    } else {
      uint8_t start = entryCol + (maxDigits - idx);
      lcd.setCursor(start, 1);
      lcd.print(tmp);
      curLen = idx;
      if (curLen == 0) {
        lastDigitPos = entryCol + maxDigits - 1;
        lastDigitChar = ' ';
      } else {
        lastDigitPos = start + curLen - 1;
        lastDigitChar = tmp[curLen - 1];
      }
    }
    if (unit) {
      lcd.setCursor(entryCol + maxDigits, 1);
      lcd.print(unit);
    }
  };

  redrawNumber(value);
  bool localEdit = editMode;
  uint32_t number = value;

  while (true) {
    char key = keypad.getKey();
    if (localEdit && millis() - lastBlink >= 500) {
      lastBlink = millis();
      showCursor = !showCursor;
      lcd.setCursor(lastDigitPos, 1);
      if (showCursor) lcd.print('|');
      else lcd.print(lastDigitChar);
    }
    if (!key) {
      delay(10);
      continue;
    }
    if (!localEdit) {
      if (key == '#') {
        localEdit = true;
        if (number > 0 && number != (uint32_t)-1) digitsEntered = true;
        else {
          digitsEntered = false;
          number = 0;
        }
        redrawNumber(number);
      } else if (key == '*') {
        return -1;
      }
      continue;
    }
    if (key == '*') {
      if (!digitsEntered || number == 0) return -1;
      number = 0;
      digitsEntered = false;
      redrawNumber(number);
      continue;
    }
    if (key == '#') {
      if (!digitsEntered || number == 0) return -1;
      return (int32_t)number;
    }
    if (key >= '0' && key <= '9') {
      if (curLen < maxDigits) {
        digitsEntered = true;
        number = number * 10 + (key - '0');
        redrawNumber(number);
      } else {
        number = (uint32_t)-1;
        return -1;
      }
      continue;
    }
  }
}

int32_t tankVolumeScreen(const char* tankVolumeBuf, bool editMode, uint32_t tankVolume) {
  Serial.print("[TANK] entry: editMode=");
  Serial.print(editMode);
  Serial.print(" tankVolume=");
  Serial.println(tankVolume);
  if (tankVolume == (uint32_t)-1ULL) {
    Serial.println("[TANK] sentinel passed -> start edit mode at 0");
    tankVolume = 0;
    editMode = true;
  }
  return editNumberScreen(tankVolumeBuf, LCD_TANK_FORMAT, 4, 7, tankVolume, editMode);
}

int16_t velocityScreen(const char* velocityBuf, int pumpIndex, bool editMode, uint32_t velocity) {
  Serial.print("[VEL] entry: editMode=");
  Serial.print(editMode);
  Serial.print(" velocity=");
  Serial.print(velocity);
  Serial.print(" index=");
  Serial.println(pumpIndex);
  char _velBuf[LANG_VELOCITYTITLE_LEN];
  strncpy(_velBuf, velocityBuf, sizeof(_velBuf)-1);
  _velBuf[sizeof(_velBuf)-1] = '\0';
  for (int i = 0; _velBuf[i] != '\0'; i++) {
    if (_velBuf[i] == '#') {
      _velBuf[i] = '1' + pumpIndex;
      break;
    }
  }
  if (velocity == (uint32_t)-1ULL) {
    Serial.println("[VEL] sentinel passed -> start edit mode at 0");
    velocity = 0;
    editMode = true;
  }
  return editNumberScreen(_velBuf, LCD_VEL_FORMAT, 4, 4, velocity, editMode);
}

void lcdPrintWithGlyphs(const char* str, uint8_t length) {
  for (uint8_t i = 0; i < length; ++i) {
    lcd.write((uint8_t)str[i]);
  }
}

uint64_t timeSetupScreen() {
  uint64_t nowSecs = seconds();
  uint32_t tod = (uint32_t)(nowSecs % 86400ULL);
  uint8_t hh = tod / 3600;
  uint8_t mm = (tod % 3600) / 60;
  uint8_t ss = tod % 60;

  auto showTime = [&](const char digits[6]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(digits[0]);
    lcd.print(digits[1]);
    lcd.print(':');
    lcd.print(digits[2]);
    lcd.print(digits[3]);
    lcd.print(':');
    lcd.print(digits[4]);
    lcd.print(digits[5]);
    lcd.setCursor(0, 1);
    lcd.print("#=OK  *=Cancel");
  };

  char digits[7];
  digits[0] = '0' + (hh / 10);
  digits[1] = '0' + (hh % 10);
  digits[2] = '0' + (mm / 10);
  digits[3] = '0' + (mm % 10);
  digits[4] = '0' + (ss / 10);
  digits[5] = '0' + (ss % 10);
  digits[6] = '\0';

  showTime(digits);

  uint8_t pos = 0;
  unsigned long lastBlink = millis();
  bool showCursor = true;
  showTime(digits);

  while (true) {
    for (uint8_t i = 0; i < 6; ++i) {
      uint8_t col = (i < 2) ? i : ((i < 4) ? (i + 1) : (i + 2));
      lcd.setCursor(col, 0);
      if (i == pos && showCursor) {
        lcd.print('|');
      } else {
        lcd.print(digits[i]);
      }
    }

    if (millis() - lastBlink >= 500) {
      lastBlink = millis();
      showCursor = !showCursor;
    }

    char key = keypad.getKey();
    if (!key) {
      delay(30);
      continue;
    }

    if (key >= '0' && key <= '9') {
      digits[pos] = key;
      pos = (pos + 1) % 6;
      showCursor = true;
      continue;
    }

    if (key == 'A') {
      pos = (pos + 1) % 6;
      continue;
    }
    if (key == 'B') {
      pos = (pos + 6 - 1) % 6;
      continue;
    }
    if (key == '*') {
      Serial.println("[TIME] edit cancelled");
      return (uint32_t)-1;
    }
    if (key == '#') {
      uint8_t nh = (digits[0] - '0') * 10 + (digits[1] - '0');
      uint8_t nm = (digits[2] - '0') * 10 + (digits[3] - '0');
      uint8_t ns = (digits[4] - '0') * 10 + (digits[5] - '0');
      if (nh > 23) nh = nh % 24;
      if (nm > 59) nm = nm % 60;
      if (ns > 59) ns = ns % 60;

      uint32_t enteredSeconds = (uint32_t)nh * 3600UL + (uint32_t)nm * 60UL + (uint32_t)ns;
      uint64_t tmptimeoffset = seconds() - (uint64_t)enteredSeconds;
      Serial.print("[TIME] entered HH:MM:SS = ");
      Serial.print(nh);
      Serial.print(":");
      Serial.print(nm);
      Serial.print(":");
      Serial.println(ns);
      Serial.print("[TIME] tmptimeoffset = ");
      Serial.println((uint32_t)tmptimeoffset);
      return (seconds() - tmptimeoffset);
    }
  }
}

void showTime(uint64_t currentTime) {
  uint32_t tod = currentTime % 86400UL;
  uint8_t hh = tod / 3600;
  uint8_t mm = (tod % 3600) / 60;
  uint8_t ss = tod % 60;

  lcd.clear();
  lcd.setCursor(0, 0);

  if (hh < 10) lcd.print('0');
  lcd.print(hh);
  lcd.print(':');

  if (mm < 10) lcd.print('0');
  lcd.print(mm);
  lcd.print(':');

  if (ss < 10) lcd.print('0');
  lcd.print(ss);
}

