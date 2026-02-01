#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include <ESPmDNS.h>
#include "Config.h"
#include "NetworkManager.h"
#include "DisplayManager.h"
#include "WebPage.h"

class WebManager {
public:
  WebManager();
  void begin();
  void update();
  
  void startAP();
  void stopAP();
  void loadSettings();
  
  void handleReset(); // Called from main loop
  
  // Accessors for auth
  String authUser = "admin";
  String authPass = "admin";

private:
  WebServer server;
  bool isServerStarted = false;
  
  // Route handlers
  void handleRoot();
  void handleScan();
  void handleConnectWifi();
  void handleListWifi();
  void handleDelWifi();
  void handleUpdateFW();
  void handleSave();
  // handleReset moved to public
  void handleReboot();
  void handleUpdate();
  void handleApiQR();
  void handleApiInfo();
  void handleApiAccounts();
  void handleApiMqttStatus();
  void handleExport();
  
  // Helpers
  bool checkAuth();
};

extern WebManager webManager;

#endif
