#include "screens.h"
#include "display.h"
#include "language.h"

struct Pump {
  uint16_t velocity = 0;

  int32_t edit(uint8_t pumpIndex, const char* velocityTitle) {
    return velocityScreen(velocityTitle, pumpIndex, true, velocity);
  }

  int32_t viewEdit(uint8_t pumpIndex, const char* velocityTitle) {
    return velocityScreen(velocityTitle, pumpIndex, false, velocity);
  }

  void setVelocity(uint16_t v) { velocity = v; }
  uint16_t getVelocity() const { return velocity; }
};

namespace AppState {
  uint8_t languageIndex = 0;
  Pump pumps[PUMP_COUNT];
  uint32_t tankVolume = 0;
  int64_t timeOffset = 0;
}


void setup() {
  Serial.begin(9600);
  Serial.println("[SETUP] Serial started");

  Serial.println("[SETUP] showing splash screen");
  splashScreen();
  Serial.println("[SETUP] running language config");

  AppState::languageIndex = langConfigScreen(AppState::languageIndex, true);
  Serial.print("[SETUP] languageIndex = ");
  Serial.println(AppState::languageIndex);

  // Pobierz tylko potrzebne pole tankVolumeTitle
  char tankTitle[LANG_TANKTITLE_LEN+1];
  readLanguageField(AppState::languageIndex, offsetof(Language, tankVolumeTitle), tankTitle, LANG_TANKTITLE_VISIBLE);

  Serial.println("[SETUP] reading tank volume");
  int32_t tv = tankVolumeScreen(tankTitle, true, AppState::tankVolume);
  Serial.print("[SETUP] tankVolume = ");
  Serial.println(tv);
  if (tv > 0) AppState::tankVolume = (uint32_t)tv;

  // Pobierz tylko potrzebne pole velocityTitle
  char velocityTitle[LANG_VELOCITYTITLE_LEN+1];
  readLanguageField(AppState::languageIndex, offsetof(Language, velocityTitle), velocityTitle, LANG_VELOCITYTITLE_VISIBLE);

  Serial.println("[SETUP] reading velocities");
  for (uint8_t i = 0; i < PUMP_COUNT; ++i) {
    int32_t v = AppState::pumps[i].edit(i, velocityTitle);
    Serial.print("[SETUP] pump[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(v);
    if (v > 0) AppState::pumps[i].setVelocity((uint16_t)v);
  }
  AppState::timeOffset = timeSetupScreen();

  lcd.clear();
}

namespace Screen {
void handleEditVelocity(uint8_t idx) {
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.print("[VEL] enter handleEditVelocity idx=");
  Serial.println(idx);
  char velocityTitle[LANG_VELOCITYTITLE_LEN+1];
  readLanguageField(AppState::languageIndex, offsetof(Language, velocityTitle), velocityTitle, LANG_VELOCITYTITLE_VISIBLE);
  int32_t v = AppState::pumps[idx].viewEdit(idx, velocityTitle);
  Serial.print("[VEL] current (view) idx=");
  Serial.print(idx);
  Serial.print(" -> ");
  Serial.println(v);
  if (v > 0) AppState::pumps[idx].setVelocity((uint16_t)v);

  unsigned long start = millis();
  char follow = 0;
  while (millis() - start < 2000) {
    follow = keypad.getKey();
    if (follow) break;
    delay(10);
  }
  Serial.print("[VEL] followup key for idx=");
  Serial.print(idx);
  Serial.print(" -> ");
  Serial.println(follow);
  if (follow == '#') {
    Serial.print("[VEL] entering edit mode idx=");
    Serial.println(idx);
    int32_t nv = AppState::pumps[idx].edit(idx, velocityTitle);
    Serial.print("[VEL] edited idx=");
    Serial.print(idx);
    Serial.print(" -> ");
    Serial.println(nv);
    if (nv > 0) AppState::pumps[idx].setVelocity((uint16_t)nv);
  }
  lcd.clear();
}

void handleEditTankVolume() {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  Serial.println("[TANK] enter handleEditTankVolume");
  char tankTitle[LANG_TANKTITLE_LEN+1];
  readLanguageField(AppState::languageIndex, offsetof(Language, tankVolumeTitle), tankTitle, LANG_TANKTITLE_VISIBLE);
  int32_t tv = tankVolumeScreen(tankTitle, false, AppState::tankVolume);
  Serial.print("[TANK] current (view) -> ");
  Serial.println(tv);
  if (tv > 0) AppState::tankVolume = (uint32_t)tv;

  unsigned long start = millis();
  char follow = 0;
  while ((millis() - start) < 2000) {
    follow = keypad.getKey();
    if (follow) break;
    delay(10);
  }
  Serial.print("[TANK] followup key -> ");
  Serial.println(follow);
  if (follow == '#') {
    Serial.println("[TANK] entering edit mode");
    int32_t ntv = tankVolumeScreen(tankTitle, true, AppState::tankVolume);
    Serial.print("[TANK] edited -> ");
    Serial.println(ntv);
    if (ntv > 0) AppState::tankVolume = (uint32_t)ntv;
  }
  lcd.clear();
}
} // namespace Screen

void loop() {
  // Pobierz tylko potrzebne pola mainScreen i noTask
  char mainScreen[LANG_MAINSCREEN_LEN+1];
  char noTask[LANG_NOTASK_LEN+1];
  readLanguageField(AppState::languageIndex, offsetof(Language, mainScreen), mainScreen, LANG_MAINSCREEN_VISIBLE);
  readLanguageField(AppState::languageIndex, offsetof(Language, noTask), noTask, LANG_NOTASK_VISIBLE);

  lcd.setCursor(0, 0);
  lcd.print(mainScreen);
  lcd.setCursor(0, 1);
  lcd.print(noTask);

  char k = keypad.getKey();
  if (k) {
    Serial.print("[LOOP] key -> ");
    Serial.println(k);
  }

  if (k >= '1' && k <= '0' + PUMP_COUNT) {
    uint8_t idx = k - '1';
    if (idx < PUMP_COUNT) {
      Serial.print("[LOOP] dispatch velocity ");
      Serial.println(idx);
      Screen::handleEditVelocity(idx);
    }
  } else if (k == 'D') {
    Serial.println("[LOOP] dispatch D -> tank volume");
    Screen::handleEditTankVolume();
  } else if (k == '0') {
    Serial.println("[LOOP] Showing Current Time");
    showTime(seconds() + AppState::timeOffset);
    delay(1000);
  } else if (k == '*') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fabric Setting?");
    lcd.setCursor(0, 1);
    lcd.print("#=Yes  *=No");
    while (true) {
      char key = keypad.getKey();
      if (key == '#') {
        Serial.println("[LOOP] factory reset confirmed");
        delay(100);
        setup();
        break;
      } else if (key == '*') {
        Serial.println("[LOOP] factory reset cancelled");
        break;
      }
      delay(10);
    }
  }

  delay(100);
}
