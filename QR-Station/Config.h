#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>
#include "driver/gpio.h"

// --- Pin Definitions (ESP32-C3 Super Mini) ---
#define TFT_CS    10
#define TFT_RST   9
#define TFT_DC    8
#define TFT_SDA   5
#define TFT_SCL   4
#define TFT_BL    7

#define BUTTON1   1
#define BUTTON2   2
#define BUTTON3   3
#define BUTTON4   6

// --- Constants ---
const int PWM_CHANNEL = TFT_BL;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;
const int QR_TOP_MARGIN = 5;
const unsigned long INACTIVITY_TIMEOUT_MS = 120000UL;

// --- Data Structures ---
struct BankAccount {
  char bin[10];
  char accNum[20];
  char bankName[30];
  char ownerName[50];
};

struct DynamicQR {
  String bin;
  String acc;
  String amount;
  String name;
  String desc;
  bool active = false;
  unsigned long startTime = 0;
};

// --- Shared Globals (External) ---
// These will be defined in the main .ino or a specific cpp file
extern BankAccount accounts[3];
extern Preferences preferences;
extern bool isPowerOn;
extern DynamicQR dynamicQR;
extern int currentMode;

#endif
