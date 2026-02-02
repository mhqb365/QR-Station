#include "DisplayManager.h"

DisplayManager displayManager;

// Constructor: Initializes the TFT object
DisplayManager::DisplayManager() : tft(TFT_CS, TFT_DC, TFT_RST), qrcode(&tft) {
}

void DisplayManager::begin() {
  pinMode(TFT_BL, OUTPUT);
  ledcAttach(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(PWM_CHANNEL, 0); 
  
  SPI.begin(TFT_SCL, -1, TFT_SDA, TFT_CS); 
  tft.initR(INITR_BLACKTAB);
  
  qrcode.init();
  qrcode.setTopMargin(QR_TOP_MARGIN);
  
  loadBrightnessSettings();
}

void DisplayManager::update() {
  // Handle notification timeout
  if (isShowingNotification && (millis() - notificationStart > 8000)) {
    isShowingNotification = false;
    displayMode(currentMode);
    Serial.println("Notification closed");
  }

  // Handle brightness indicator timeout
  if (isShowingBrightness && (millis() - brightnessIndicatorStart > 2000)) {
    isShowingBrightness = false;
    displayMode(currentMode);
  }

  // Update status dots periodically
  static unsigned long lastStatusUpdate = 0;
  if (isPowerOn && !isShowingNotification && (millis() - lastStatusUpdate > 2000)) {
    lastStatusUpdate = millis();
    drawStatusDots();
  }
}

void DisplayManager::showSplashScreen() {
  tft.fillScreen(ST77XX_BLACK);
  ledcWrite(PWM_CHANNEL, currentBrightness);

  tft.drawCircle(64, 80, 40, 0x2104);
  tft.drawCircle(64, 80, 50, 0x1022);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW);

  int16_t x, y;
  uint16_t w, h;
  String title = "QR Station";
  tft.getTextBounds(title, 0, 0, &x, &y, &w, &h);
  tft.setCursor((128 - w) / 2, 60);
  tft.print(title);

  for(int i=0; i<10; i++) {
    tft.fillRect(34, 100, 60, 4, ST77XX_BLACK);
    tft.drawRect(34, 100, 60, 4, 0x5AEB);       
    tft.fillRect(34, 100, (i+1)*6, 4, 0x00D2FF); 
    delay(200);
  }
}

void DisplayManager::displayMode(int mode) {
  if (mode == -1 && dynamicQR.active) {
    ledcWrite(PWM_CHANNEL, 255);
    tft.fillScreen(ST77XX_WHITE);
    displayBankQR(
      generateQRString(dynamicQR.bin, dynamicQR.acc, dynamicQR.amount, dynamicQR.desc),
      "",
      dynamicQR.amount + " VND",
      dynamicQR.name
    );
    drawStatusDots();
    return;
  }

  ledcWrite(PWM_CHANNEL, brightnessLevels[currentBrightnessIndex]);
  
  int idx = mode - 1;
  if (idx < 0 || idx > 2) idx = 0;
  
  if (strlen(accounts[idx].accNum) == 0) {
    displayEmptyMessage();
    return;
  }

  tft.fillScreen(ST77XX_WHITE);
  displayBankQR(
    generateQRString(accounts[idx].bin, accounts[idx].accNum),
    accounts[idx].bankName,
    accounts[idx].accNum,
    accounts[idx].ownerName
  );
  drawStatusDots();
}

void DisplayManager::displayBankQR(const String &qrText, const String &bankName, const String &accountNumber, const String &accountName) {
  int qrSize = 100;
  int qrTopMargin = 22;
  qrcode.setTopMargin(qrTopMargin); 
  qrcode.create(qrText);
  
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(false);
  
  int currentY = 5; 
  printCenteredWrapped(bankName, 15, currentY);
  
  currentY = qrTopMargin + qrSize + 1; 
  printCenteredWrapped(accountNumber, 15, currentY);
  currentY += 2;
  printCenteredWrapped(accountName, 15, currentY);
  
  tft.setTextWrap(true);
}

void DisplayManager::showNotification(long amount, const char* content, const char* gateway, const char* account) {
  if (!isPowerOn) {
    tft.sendCommand(0x29);
    isPowerOn = true;
  }
  ledcWrite(PWM_CHANNEL, 255); // Max brightness
  
  tft.fillScreen(ST77XX_WHITE);
  tft.fillRect(0, 0, 128, 25, 0x03E0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds("GIAO DICH MOI", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((128 - w) / 2, 8);
  tft.print("GIAO DICH MOI");

  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  String amtStr = "+" + String(amount);
  tft.getTextBounds(amtStr, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((128 - w) / 2, 45);
  tft.print(amtStr);

  tft.setTextSize(1);
  tft.setTextColor(0x0000);
  String bankInfo = String(gateway);
  if (strlen(account) > 0) bankInfo += " (" + String(account) + ")";
  
  if (bankInfo.length() > 0) {
    tft.getTextBounds(bankInfo, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((128 - w) / 2, 75);
    tft.print(bankInfo);
  }

  tft.setTextColor(0x5AEB);
  int contentY = 95;
  printCenteredWrapped(String(content), 15, contentY);

  isShowingNotification = true;
  notificationStart = millis();
}

void DisplayManager::drawStatusDots() {
  if (!isPowerOn || isShowingNotification) return;

  int idx = currentMode - 1;
  uint16_t bgColor = ST77XX_WHITE;
  if (!dynamicQR.active && strlen(accounts[idx].accNum) == 0) bgColor = ST77XX_BLACK;

  // WiFi
  uint16_t wifiColor = (networkManager.isWifiConnected()) ? ST77XX_GREEN : ST77XX_RED;
  const int16_t W = 20;
  const int16_t H = 12;
  int16_t wx_start = 0, wy_start = 2;
 
  tft.fillRect(wx_start, wy_start, W, H, bgColor);

  const int16_t cx = wx_start + W / 2;
  const int16_t cy = wy_start + H - 2;

  int16_t radii[] = {9, 6, 3};
  for (int i = 0; i < 3; i++) {
    int16_t r = radii[i];
    for (int deg = 225; deg <= 315; deg += 3) {
      float rad = deg * 0.0174532925f;
      int16_t px = cx + (int16_t)(cos(rad) * r);
      int16_t py = cy + (int16_t)(sin(rad) * r);
      tft.drawPixel(px, py, wifiColor);
    }
  }
  tft.drawPixel(cx, cy, wifiColor);

  // MQTT
  uint16_t mqttColor = (networkManager.isMqttConnected()) ? ST77XX_GREEN : ST77XX_RED;
  int mx = 120, my = 8;
  tft.fillRect(mx - 3, my - 3, 7, 7, bgColor);
  tft.fillCircle(mx, my, 3, mqttColor);
}

void DisplayManager::displayEmptyMessage() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(10, 30);
  tft.println("Empty QR");
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 110);
  tft.println("Hold K4 to setup");
  drawStatusDots();
}

void DisplayManager::togglePower() {
  if (isPowerOn) {
    ledcWrite(PWM_CHANNEL, 0);
    tft.sendCommand(0x28);
    isPowerOn = false;
    Serial.println("Power OFF");
  } else {
    togglePowerOnOnly();
  }
}

void DisplayManager::togglePowerOnOnly() {
  tft.sendCommand(0x29);
  ledcWrite(PWM_CHANNEL, currentBrightness);
  isPowerOn = true;
  Serial.println("Power ON");
}

void DisplayManager::loadBrightnessSettings() {
  currentBrightnessIndex = preferences.getInt("brightness", 4);
  if (currentBrightnessIndex < 0 || currentBrightnessIndex > 4) currentBrightnessIndex = 4;
  currentBrightness = brightnessLevels[currentBrightnessIndex];
}

void DisplayManager::saveBrightness() {
  preferences.begin("bank_data", false);
  preferences.putInt("brightness", currentBrightnessIndex);
  preferences.end();
}

void DisplayManager::setBrightness(int brightness) {
  currentBrightness = brightness;
  if (isPowerOn) {
    ledcWrite(PWM_CHANNEL, currentBrightness);
  }
}

void DisplayManager::increaseBrightness() {
  if (currentBrightnessIndex < 4) {
    currentBrightnessIndex++;
    setBrightness(brightnessLevels[currentBrightnessIndex]);
    saveBrightness();
  }
}

void DisplayManager::decreaseBrightness() {
  if (currentBrightnessIndex > 0) {
    currentBrightnessIndex--;
    setBrightness(brightnessLevels[currentBrightnessIndex]);
    saveBrightness();
  }
}

void DisplayManager::showBrightnessIndicator() {
  if (!isPowerOn) return;

  isShowingBrightness = true;
  brightnessIndicatorStart = millis();

  int barWidth = 80;
  int barHeight = 10;
  int barX = (128 - barWidth) / 2;
  int barY = 60;
  
  tft.fillRect(barX - 2, barY - 2, barWidth + 4, barHeight + 4, ST77XX_BLACK);
  tft.drawRect(barX - 2, barY - 2, barWidth + 4, barHeight + 4, ST77XX_WHITE);
  
  int fillWidth = (barWidth * (currentBrightnessIndex + 1)) / 5;
  tft.fillRect(barX, barY, barWidth, barHeight, ST77XX_BLACK);
  tft.fillRect(barX, barY, fillWidth, barHeight, ST77XX_CYAN);
  
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  String percentText = String((currentBrightnessIndex + 1) * 20) + "%";
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(percentText, 0, 0, &x1, &y1, &w, &h);
  tft.fillRect((128 - w) / 2 - 2, barY + barHeight + 5 - 2, w + 4, h + 4, ST77XX_BLACK);
  tft.setCursor((128 - w) / 2, barY + barHeight + 5);
  tft.print(percentText);
}

void DisplayManager::handleIPDisplay() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);
  
  tft.setCursor(10, 20);
  tft.println("SYSTEM INFO");
  
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 40);
  tft.println("Device IP");
  
  if (networkManager.isWifiConnected()) {
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 50);
    tft.println(WiFi.localIP().toString());
  } else {
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 50);
    tft.println("Not connect WIFI");
  }
}

void DisplayManager::handleConfigModeDisplay() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);
  
  tft.setCursor(10, 20);
  tft.println("CONFIG MODE");
  
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 40);
  tft.println("Connect WiFi");
  
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 50);
  tft.println("SSID: QR Station");
  tft.setCursor(10, 60);
  tft.println("Pass: 88888888");

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 75);
  tft.println("Access to");
  
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 85);
  tft.println("192.168.4.1");
}

bool DisplayManager::isContentVisible() {
  return isPowerOn;
}

// Helpers
uint16_t DisplayManager::crc16_ccitt(const char* data, int length) {
  uint16_t crc = 0xFFFF;
  for (int i = 0; i < length; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
      else crc <<= 1;
    }
  }
  return crc;
}

String DisplayManager::fLen(int len) {
  if (len < 10) return "0" + String(len);
  return String(len);
}

String DisplayManager::generateQRString(String bin, String account, String amount, String desc) {
  String sub01 = "0006" + bin + "01" + fLen(account.length()) + account;
  String tag38Content = String("0010A000000727") + "01" + fLen(sub01.length()) + sub01 + "0208QRIBFTTA";
  
  String qr = "000201";
  if (amount != "" && amount != "0") qr += "010212";
  else qr += "010211";
  
  qr += "38" + fLen(tag38Content.length()) + tag38Content;
  qr += "5303704";
  if (amount != "" && amount != "0") qr += "54" + fLen(amount.length()) + amount;
  qr += "5802VN";
  if (desc != "") {
    String tag62Content = "08" + fLen(desc.length()) + desc;
    qr += "62" + fLen(tag62Content.length()) + tag62Content;
  }
  qr += "6304";
  
  char crcStr[5];
  sprintf(crcStr, "%04X", crc16_ccitt(qr.c_str(), qr.length()));
  qr += String(crcStr);
  return qr;
}

void DisplayManager::printCenteredWrapped(const String &text, int maxChars, int &currentY) {
  int screenWidth = 128;
  String name = text;
  name.trim(); // Just simplistic wrapper, same as original
  
  // Reuse logic from original
  String currentLine = "";
  int i = 0;
  while (i < name.length()) {
    int nextSpace = name.indexOf(' ', i);
    int wordEnd = (nextSpace == -1) ? name.length() : nextSpace;
    String word = name.substring(i, wordEnd);
    if (word.length() == 0) {
      i = wordEnd + 1;
      continue;
    }
    String testLine = (currentLine == "") ? word : currentLine + " " + word;
    if (testLine.length() > maxChars) {
      if (currentLine != "") {
        int16_t tx, ty; uint16_t tw, th;
        tft.getTextBounds(currentLine, 0, 0, &tx, &ty, &tw, &th);
        tft.setCursor((screenWidth - tw) / 2, currentY);
        tft.print(currentLine);
        currentY += th + 3;
        currentLine = word;
      } else {
        int16_t tx, ty; uint16_t tw, th;
        tft.getTextBounds(word, 0, 0, &tx, &ty, &tw, &th);
        tft.setCursor((screenWidth - tw) / 2, currentY);
        tft.print(word);
        currentY += th + 3;
        currentLine = "";
      }
    } else {
      currentLine = testLine;
    }
    i = wordEnd + 1;
  }
  if (currentLine.length() > 0) {
    int16_t tx, ty; uint16_t tw, th;
    tft.getTextBounds(currentLine, 0, 0, &tx, &ty, &tw, &th);
    tft.setCursor((screenWidth - tw) / 2, currentY);
    tft.print(currentLine);
    currentY += th + 3;
  }
}
