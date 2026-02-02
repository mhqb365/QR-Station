/*
 * Project: QR Station
 * Open Source: https://github.com/mhqb365/QR-Station
 * Developer: mhqb365.com
 * Refactored by Antigravity (Google DeepMind)
 */

#include "Config.h"
#include "NetworkManager.h"
#include "DisplayManager.h"
#include "WebManager.h"
#include "ButtonManager.h"

// --- Global Variables (Instantiated here) ---
BankAccount accounts[3];
Preferences preferences;
bool isPowerOn = true;
DynamicQR dynamicQR;
int currentMode = 1;
unsigned long lastBrightnessAction = 0;

// --- Callback Functions for Buttons ---

void onK1Click() {
  if (digitalRead(BUTTON2) == LOW || (millis() - lastBrightnessAction < 500)) return;
  
  if (dynamicQR.active || displayManager.isShowingNotification) {
      dynamicQR.active = false;
      displayManager.isShowingNotification = false;
      if(!isPowerOn) displayManager.togglePowerOnOnly();
      displayManager.displayMode(currentMode);
      return;
  }
  
  if (!isPowerOn) {
      displayManager.togglePowerOnOnly();
      return;
  }

  // Normal mode switch or other logic?
  // Original logic: K1 switches to Mode 1.
  currentMode = 1;
  displayManager.displayMode(1);
}

void onK1LongPress() {
    if (isPowerOn) {
        displayManager.handleIPDisplay();
    }
}

void onK2Click() {
  if (millis() - lastBrightnessAction < 500) return;

  if (dynamicQR.active || displayManager.isShowingNotification) {
      dynamicQR.active = false;
      displayManager.isShowingNotification = false;
      if(!isPowerOn) displayManager.togglePowerOnOnly();
      displayManager.displayMode(currentMode);
      return;
  }
  
  if (!isPowerOn) {
      displayManager.togglePowerOnOnly();
      return;
  }
  
  currentMode = 2;
  displayManager.displayMode(2);
}

void onK2LongPress() {
    if (isPowerOn) {
        // Reboot
        // TODO: Show nice message
        ESP.restart();
    }
}

void onK3Click() {
  if (digitalRead(BUTTON2) == LOW || (millis() - lastBrightnessAction < 500)) return;

  if (dynamicQR.active || displayManager.isShowingNotification) {
      dynamicQR.active = false;
      displayManager.isShowingNotification = false;
      if(!isPowerOn) displayManager.togglePowerOnOnly();
      displayManager.displayMode(currentMode);
      return;
  }
  
  if (!isPowerOn) {
      displayManager.togglePowerOnOnly();
      return;
  }

  currentMode = 3;
  displayManager.displayMode(3);
}

void onK3LongPress() {
    if (isPowerOn) {
        // Factory reset
        webManager.handleReset(); // Reusing the handler logic
    }
}

void onK4Click() {
    // Wake up if off
    if (!isPowerOn) {
        // Wait, double click K4 toggles power. Single click does nothing if off?
        // Original logic: if off, wait for double click check.
        // My ButtonManager handles this separately. 
        // Let's say click wakes it up or ignores?
        // Original: "Nếu màn hình tắt, nhấn nút bất kỳ chỉ để bật lên" (implicit via logic)
        // For K4, it has special double click logic.
    }
    // If on, does nothing specifically other than existing mode?
    // Actually K4 is often used for Config mode (Long press)
}

void onK4DoubleClick() {
    displayManager.togglePower();
}

void onK4LongPress() {
  if (isPowerOn) {
    WiFi.softAP("QR Station", "88888888");
    displayManager.handleConfigModeDisplay();
    webManager.startAP();
  }
}


// --- MQTT Callback ---
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT Msg: "); Serial.println(topic);
  
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, payload, length);
  
  if (error) {
    Serial.println("JSON Error");
    return;
  }
  
  long amount = 0;
  const char* content = "No content";
  const char* gateway = "";
  const char* account = "";

  // Logic adapted from original
  if (doc.containsKey("transactions") && doc["transactions"].is<JsonArray>()) {
    JsonArray transactions = doc["transactions"].as<JsonArray>();
    for (JsonObject trans : transactions) {
       amount = trans["transferAmount"] | trans["amount"] | 0;
       content = trans["content"] | trans["description"] | trans["desc"] | "No content";
       gateway = trans["gateway"] | "";
       account = trans["accountNumber"] | "";
       if (amount > 0) displayManager.showNotification(amount, content, gateway, account);
    }
    return;
  } 
  
  if (doc.containsKey("amount") || doc.containsKey("transferAmount")) {
    amount = doc["amount"] | doc["transferAmount"] | 0;
    content = doc["content"] | doc["description"] | doc["desc"] | "No content";
    gateway = doc["gateway"] | "";
    account = doc["accountNumber"] | "";
  } else if (doc.is<JsonArray>()) {
     // Handle array root... simply pick first? Original looped.
     JsonArray arr = doc.as<JsonArray>();
     for (JsonObject trans : arr) {
       amount = trans["amount"] | trans["transferAmount"] | 0;
       content = trans["content"] | trans["description"] | trans["desc"] | "No content";
       gateway = trans["gateway"] | "";
       account = trans["accountNumber"] | "";
       if (amount > 0) displayManager.showNotification(amount, content, gateway, account);
     }
     return;
  }

  if (amount > 0) {
    displayManager.showNotification(amount, content, gateway, account);
  }
}

// --- Setup & Loop ---

void setup() {
  Serial.begin(115200);
  
  buttonManager.begin();
  displayManager.begin();
  networkManager.begin();
  webManager.begin();
  
  // Setup Buttons
  buttonManager.setClickHandler(0, onK1Click);
  buttonManager.setLongPressHandler(0, onK1LongPress);
  
  buttonManager.setClickHandler(1, onK2Click);
  buttonManager.setLongPressHandler(1, onK2LongPress);
  // Note: Brightness controls on K2 were complex (K2 hold + K1/K3). 
  // For now simplified to direct buttons? 
  // Or we need logic in loop to handle "While K2 held".
  // ButtonManager doesn't support "While Pressed" easily yet without modification.
  // We can add raw check in loop for brightness.
  
  buttonManager.setClickHandler(2, onK3Click);
  buttonManager.setLongPressHandler(2, onK3LongPress);
  
  buttonManager.setClickHandler(3, onK4Click);
  buttonManager.setDoubleClickHandler(3, onK4DoubleClick);
  buttonManager.setLongPressHandler(3, onK4LongPress);
  
  networkManager.setMqttCallback(onMqttMessage);
  
  displayManager.showSplashScreen();
  displayManager.displayMode(currentMode);
}

void loop() {
  buttonManager.update();
  networkManager.update();
  webManager.update();
  displayManager.update();

  // Handling Dynamic QR Timeout
  if (isPowerOn && dynamicQR.active && (millis() - dynamicQR.startTime >= 60000UL)) {
    dynamicQR.active = false;
    displayManager.displayMode(currentMode);
  }
  
  // Brightness Control Logic (Legacy K2 hold + K1/K3)
  if (digitalRead(BUTTON2) == LOW) {
      if (digitalRead(BUTTON1) == LOW) {
          lastBrightnessAction = millis();
          buttonManager.setHandled(0); // Mark K1 handled
          buttonManager.setHandled(1); // Mark K2 handled
          displayManager.increaseBrightness();
          displayManager.showBrightnessIndicator();
          delay(150); 
      }
      if (digitalRead(BUTTON3) == LOW) {
          lastBrightnessAction = millis();
          buttonManager.setHandled(2); // Mark K3 handled
          buttonManager.setHandled(1); // Mark K2 handled
          displayManager.decreaseBrightness();
          displayManager.showBrightnessIndicator();
          delay(150);
      }
  }

  yield();
}