#include "NetworkManager.h"

QRNetworkManager networkManager;

QRNetworkManager::QRNetworkManager() : mqttClient(espClient) {
}

void QRNetworkManager::begin() {
  loadSettings();
  setupWiFi();
}

void QRNetworkManager::update() {
  if (connectionDisabled) return;

  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      reconnectMQTT();
    }
    // Disconnect MQTT if manually disabled
    if (!mqttEnabled && mqttClient.connected()) {
      mqttClient.disconnect();
      Serial.println("MQTT disabled, disconnecting...");
    }
    mqttClient.loop();
  } else {
    // Retry WiFi
     if (millis() - lastWifiRetry > 15000) { 
        lastWifiRetry = millis();
        Serial.println("WiFi disconnected, searching for saved networks...");
        wifiMulti.run();
      }
  }
}

void QRNetworkManager::setupWiFi() {
  connectionDisabled = false;
  wifiRetries = 0;
  mqttRetries = 0;
  lastWifiRetry = millis();

  WiFi.mode(WIFI_STA);
  
  StaticJsonDocument<1024> doc;
  deserializeJson(doc, savedWifiList);
  JsonArray arr = doc.as<JsonArray>();
  
  if (arr.size() == 0) {
    Serial.println("No saved WiFi networks. Skipping connection");
    return;
  }
  
  for (JsonObject obj : arr) {
    const char* s = obj["s"];
    const char* p = obj["p"];
    wifiMulti.addAP(s, p);
    Serial.print("Added AP: "); Serial.println(s);
  }
  
  Serial.println("Connecting to best available WiFi...");
  wifiMulti.run();
}

void QRNetworkManager::reconnectMQTT() {
  if (!mqttEnabled) return;
  if (connectionDisabled) return;
  if (mqttServer.length() == 0) return;
  if (WiFi.status() != WL_CONNECTED) return;

  static unsigned long mqttRetryInterval = 5000;

  if (!mqttClient.connected()) {
    if (millis() - lastMqttRetry > mqttRetryInterval) {
      lastMqttRetry = millis();
      Serial.print("Attempting MQTT connection...");
      
      String host = mqttServer;
      int port = 1883;
      int colonIndex = mqttServer.indexOf(':');
      if (colonIndex != -1) {
        host = mqttServer.substring(0, colonIndex);
        port = mqttServer.substring(colonIndex + 1).toInt();
      }

      mqttClient.setServer(host.c_str(), port);
      mqttClient.setBufferSize(2048);
      
      String clientId = "qr_station_";
      clientId += String(random(0xffff), HEX);
      
      bool connected = false;
      if (mqttUser.length() > 0) {
        connected = mqttClient.connect(clientId.c_str(), mqttUser.c_str(), mqttPass.c_str());
      } else {
        connected = mqttClient.connect(clientId.c_str());
      }

      if (connected) {
        Serial.println("connected");
        mqttClient.subscribe(mqtt_topic);
        mqttClient.subscribe("transfers"); 
        mqttRetries = 0;
        mqttRetryInterval = 5000;
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again later");
        mqttRetries++;
        if (mqttRetries >= MAX_RETRIES) {
             mqttRetryInterval = 30000;
             mqttRetries = 0;
        }
      }
    }
  }
}

bool QRNetworkManager::isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool QRNetworkManager::isMqttConnected() {
  return mqttClient.connected();
}

void QRNetworkManager::setMqttCallback(MqttCallbackType callback) {
  mqttClient.setCallback(callback);
}

void QRNetworkManager::addWifi(const char* ssid, const char* pass) {
  wifiMulti.addAP(ssid, pass);
}

void QRNetworkManager::loadSettings() {
  preferences.begin("bank_data", true); // Use read-only for loading
  
  String s = preferences.getString("w_ssid", "");
  String p = preferences.getString("w_pass", "");
  String m = preferences.getString("m_serv", "");
  String mu = preferences.getString("m_user", "");
  String mp = preferences.getString("m_pass", "");
  
  bool oldEn = mqttEnabled;
  String oldServ = mqttServer;
  
  mqttEnabled = preferences.getBool("m_en", true);
  savedWifiList = preferences.getString("w_list", "[]");

  // Migration logic
  if (s != "" && savedWifiList == "[]") {
    preferences.end();
    preferences.begin("bank_data", false);
    StaticJsonDocument<256> doc;
    JsonArray arr = doc.to<JsonArray>();
    JsonObject obj = arr.createNestedObject();
    obj["s"] = s;
    obj["p"] = p;
    serializeJson(doc, savedWifiList);
    preferences.putString("w_list", savedWifiList);
    preferences.end();
    preferences.begin("bank_data", true);
  }
  
  mqttServer = m;
  mqttUser = mu;
  mqttPass = mp;
  
  preferences.end();
  
  // If MQTT settings significantly changed or disabled, disconnect to force new setup
  if (mqttClient.connected()) {
      if (!mqttEnabled || mqttServer != oldServ) {
          mqttClient.disconnect();
          Serial.println("MQTT settings changed, disconnected.");
      }
  }
}

void QRNetworkManager::saveWifiList(String json) {
  preferences.begin("bank_data", false);
  preferences.putString("w_list", json);
  preferences.end();
  savedWifiList = json;
}

void QRNetworkManager::addOrUpdateWifi(String ssid, String pass) {
  StaticJsonDocument<1024> doc;
  deserializeJson(doc, savedWifiList);
  JsonArray arr = doc.as<JsonArray>();
  
  bool found = false;
  for (JsonObject obj : arr) {
    if (obj["s"] == ssid) {
      obj["p"] = pass;
      found = true;
      break;
    }
  }
  
  if (!found) {
    JsonObject obj = arr.createNestedObject();
    obj["s"] = ssid;
    obj["p"] = pass;
  }
  
  String output;
  serializeJson(doc, output);
  
  // Save both the list and the "current/last" wifi credentials for sync
  preferences.begin("bank_data", false);
  preferences.putString("w_list", output);
  preferences.putString("w_ssid", ssid);
  preferences.putString("w_pass", pass);
  preferences.end();
  
  savedWifiList = output;
}

String QRNetworkManager::getWifiPass(String ssid) {
  StaticJsonDocument<1024> doc;
  deserializeJson(doc, savedWifiList);
  JsonArray arr = doc.as<JsonArray>();
  
  for (JsonObject obj : arr) {
    if (obj["s"] == ssid) {
      return obj["p"].as<String>();
    }
  }
  return "";
}

String QRNetworkManager::getSavedWifiList() {
    return savedWifiList;
}
