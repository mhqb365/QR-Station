#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <qrcode_st7735.h>
#include "Config.h"
#include "NetworkManager.h" // For status dots

class DisplayManager {
public:
  DisplayManager();
  void begin();
  void update();
  
  void showSplashScreen();
  void displayMode(int mode);
  void showNotification(long amount, const char* content, const char* gateway, const char* account);
  void displayBankQR(const String &qrText, const String &bankName, const String &accountNumber, const String &accountName);
  void handleIPDisplay();
  
  void togglePower();
  void togglePowerOnOnly();
  void setBrightness(int brightness);
  void increaseBrightness();
  void decreaseBrightness();
  void showBrightnessIndicator();
  
  void loadBrightnessSettings();
  
  bool isContentVisible(); // true if screen is on
  bool isShowingNotification = false;
  unsigned long notificationStart = 0;
  bool isShowingBrightness = false;
  unsigned long brightnessIndicatorStart = 0;

private:
  Adafruit_ST7735 tft;
  QRcode_ST7735 qrcode;
  
  int brightnessLevels[5] = {51, 102, 153, 204, 255};
  int currentBrightnessIndex = 4;
  int currentBrightness = 255;
  
  // Helper methods
  void drawStatusDots();
  void printCenteredWrapped(const String &text, int maxChars, int &currentY);
  String generateQRString(String bin, String account, String amount = "", String desc = "");
  
  void displayEmptyMessage();
  uint16_t crc16_ccitt(const char* data, int length);
  String fLen(int len);
  void saveBrightness();
};

extern DisplayManager displayManager;

#endif
