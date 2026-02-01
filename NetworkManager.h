#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Config.h"

// Callback type for MQTT messages
typedef std::function<void(char*, byte*, unsigned int)> MqttCallbackType;

class QRNetworkManager {
public:
  QRNetworkManager();
  void begin();
  void update();
  
  void setupWiFi();
  void reconnectMQTT();
  
  bool isWifiConnected();
  bool isMqttConnected();
  
  void setMqttCallback(MqttCallbackType callback);
  void addWifi(const char* ssid, const char* pass);
  
  // Public accessors for configuration
  void loadSettings();
  void saveWifiList(String json);
  void addOrUpdateWifi(String ssid, String pass);
  
  String getSavedWifiList();
  
  // MQTT config exposure
  String mqttServer;
  String mqttUser;
  String mqttPass;
  bool mqttEnabled = true;

private:
  WiFiMulti wifiMulti;
  WiFiClient espClient;
  PubSubClient mqttClient;
  
  unsigned long lastWifiRetry = 0;
  unsigned long lastMqttRetry = 0;
  int wifiRetries = 0;
  int mqttRetries = 0;
  const int MAX_RETRIES = 5;
  
  bool connectionDisabled = false;
  String savedWifiList = "[]"; 
  
  // MQTT details
  const char* mqtt_topic = "transfers";
};

extern QRNetworkManager networkManager;

#endif
