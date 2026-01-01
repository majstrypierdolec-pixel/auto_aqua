#ifndef LANGUAGES_H
#define LANGUAGES_H
#include <stdint.h>
#include <string.h>

constexpr uint8_t LANG_COUNT = 10;
constexpr uint8_t LANG_NAME_LEN = 16;
constexpr uint8_t LANG_PROMPT_LEN = 12;
constexpr uint8_t LANG_TANKTITLE_LEN = 24;
constexpr uint8_t LANG_VELOCITYTITLE_LEN = 24;
constexpr uint8_t LANG_MAINSCREEN_LEN = 24;
constexpr uint8_t LANG_NOTASK_LEN = 24;
constexpr uint8_t LANG_PUMPWORKING_LEN = 16;

constexpr uint8_t LANG_NAME_VISIBLE = 8;
constexpr uint8_t LANG_PROMPT_VISIBLE = 7;
constexpr uint8_t LANG_TANKTITLE_VISIBLE = 16; // ile znaków wyświetlać na LCD (możesz dostosować)
constexpr uint8_t LANG_VELOCITYTITLE_VISIBLE = 16;
constexpr uint8_t LANG_MAINSCREEN_VISIBLE = 16;
constexpr uint8_t LANG_NOTASK_VISIBLE = 16;
constexpr uint8_t PUMP_COUNT = 5;

struct Language {
  char langName[LANG_NAME_LEN];
  char langPrompt[LANG_PROMPT_LEN];
  char tankVolumeTitle[LANG_TANKTITLE_LEN];
  char velocityTitle[LANG_VELOCITYTITLE_LEN];
  char mainScreen[LANG_MAINSCREEN_LEN];
  char noTask[LANG_NOTASK_LEN];
  char pumpWorking[LANG_PUMPWORKING_LEN];
};

const Language LANGUAGES[LANG_COUNT] PROGMEM = {
  { "Polski   ", "J\000zyk  ", "Poj. zbiornika  ", "Przep\001yw pompy #", "Ekran g\001\002wny    ", "Brak zadania    " },
  { "English  ", "Lang   ", "Tank volume     ", "Pump # flow     ", "Main screen     ", "No task         " },
  { "Pycckий  ", "Языk   ", "O6ъём бака      ", "Расход насоса # ", "Главный экран   ", "Нет задачи      " },
  { "Deutsch  ", "Sprache", "Tankvolumen     ", "Pumpenfluss #   ", "Hauptbildschirm ", "Keine Aufgabe   " },
  { "Français ", "Langue ", "Vol. réservoir  ", "Pompe # débit   ", "Écran principal ", "Aucune tâche    " },
  { "Español  ", "Idioma ", "Vol. tanque     ", "Bomba # flujo   ", "Pantalla princ. ", "Sin tarea       " },
  { "Italiano ", "Lingua ", "Vol. serbatoio  ", "Flusso pompa #  ", "Schermo prin.   ", "Nessun compito  " },
  { "Português", "Idioma ", "Vol. do tanque  ", "Vazão bomba #   ", "Tela principal  ", "Sem tarefa      " },
  { "Türkçe   ", "Dil    ", "Hacim           ", "Pompa Akışı #   ", "Ana ekran       ", "Görev yok       " },
  { "Čeština  ", "Jazyk  ", "Objem nádrže    ", "Průtok čerpadla 1", "Hlavní obrazovka", "Žádný úkol      " }
};

// Storage function declarations (implemented separately)
Language readLanguage(uint8_t idx);
void readLanguageField(uint8_t idx, uint8_t offset, char* dest, uint8_t len);

#endif
