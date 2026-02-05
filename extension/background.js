// Background service worker for QR Station Link
chrome.sidePanel
  .setPanelBehavior({ openPanelOnActionClick: true })
  .catch((error) => console.error(error));

var window = self;
var global = self;
var document = {
  createElement: function () {
    return {};
  },
  location: self.location,
};
var localStorage = {
  getItem: function () {
    return null;
  },
  setItem: function () {},
  removeItem: function () {},
};
var navigator = {
  userAgent: "Mozilla/5.0",
};

importScripts("mqtt.min.js");

let mqttClient = null;
let retryCount = 0;
const MAX_RETRIES = 3;

function connectMQTT() {
  chrome.storage.local.get(
    [
      "mqtt_host",
      "mqtt_port",
      "mqtt_user",
      "mqtt_pass",
      "mqtt_enabled",
      "mqtt_is_editing",
    ],
    (settings) => {
      if (settings.mqtt_enabled === false) {
        console.log("MQTT is disabled in settings");
        chrome.storage.local.set({ mqtt_status: "disconnected" });
        return;
      }

      // Skip if user is currently editing in popup
      if (settings.mqtt_is_editing) {
        console.log("MQTT auto-connect skipped: User is editing settings");
        return;
      }

      _doConnect(settings);
    },
  );
}

function _doConnect(settings) {
  if (mqttClient) {
    mqttClient.end(true); // Force end existing client
    mqttClient = null;
  }

  retryCount = 0; // Reset counter for every fresh connection call
  const host = settings.mqtt_host;
  const port = settings.mqtt_port;
  const user = settings.mqtt_user || "";
  const pass = settings.mqtt_pass || "";

  if (!host || !port) {
    console.log("MQTT settings incomplete, skipping connection");
    chrome.storage.local.set({ mqtt_status: "disconnected" });
    return;
  }

  // Use wss:// for port 443 or 8084, otherwise ws:// (User's log indicates 8883 is WS, not WSS)
  const protocol = port == "443" || port == "8084" ? "wss" : "ws";
  const url = `${protocol}://${host}:${port}`;

  console.log(
    `Connecting to MQTT (${protocol}) (Attempt ${retryCount + 1}/${MAX_RETRIES}):`,
    url,
  );

  // Clear any previous error before connecting
  chrome.storage.local.set({
    mqtt_error: null,
    mqtt_status: "connecting",
    mqtt_attempt: retryCount + 1,
  });

  mqttClient = mqtt.connect(url, {
    username: user,
    password: pass,
    reconnectPeriod: 5000,
    keepalive: 60,
    connectTimeout: 10 * 1000,
    clientId: "qr_link_" + Math.random().toString(16).substr(2, 8),
  });

  mqttClient.on("connect", () => {
    console.log("MQTT connected");
    retryCount = 0; // Reset count on success
    chrome.storage.local.set({
      mqtt_status: "connected",
      mqtt_error: null,
    });
    mqttClient.subscribe("transfers", (err) => {
      if (!err) {
        console.log("Subscribed to transfers");
      }
    });
  });

  mqttClient.on("message", (topic, message) => {
    if (topic === "transfers") {
      const payload = message.toString();
      console.log("Received MQTT message:", payload);

      chrome.storage.local.get(["mqtt_status"], (res) => {
        if (res.mqtt_status !== "connected") {
          chrome.storage.local.set({
            mqtt_status: "connected",
            mqtt_error: null,
          });
        }
      });

      // Broadcast to all tabs
      chrome.tabs.query({}, (tabs) => {
        tabs.forEach((tab) => {
          chrome.tabs
            .sendMessage(tab.id, {
              action: "mqtt-message",
              topic,
              payload,
            })
            .catch(() => {});
        });
      });

      // Play notification sound
      chrome.storage.local.get(["mqtt_sound"], (s) => {
        if (s.mqtt_sound !== false) {
          try {
            const data = JSON.parse(payload);
            playNotificationSound(data);
          } catch (e) {
            playNotificationSound(null);
          }
        }
      });
    }
  });

  mqttClient.on("reconnect", () => {
    retryCount++;
    console.log(`MQTT reconnecting... Attempt ${retryCount}/${MAX_RETRIES}`);

    if (retryCount >= MAX_RETRIES) {
      console.log("Max retries reached, giving up.");
      if (mqttClient) {
        mqttClient.end(true);
        mqttClient = null;
      }
      chrome.storage.local.set({
        mqtt_status: "error",
        mqtt_error: `Connect failed. Please check configuration.`,
      });
    } else {
      chrome.storage.local.set({
        mqtt_status: "connecting",
        mqtt_attempt: retryCount + 1,
      });
    }
  });

  mqttClient.on("close", () => {
    console.log("MQTT closed");
    if (mqttClient && !mqttClient.connected && !mqttClient.reconnecting) {
      chrome.storage.local.set({ mqtt_status: "disconnected" });
    }
  });

  mqttClient.on("error", (err) => {
    console.error("MQTT Error:", err);
    let msg = err.message;
    if (msg.includes("SSL") || msg.includes("protocol")) {
      msg += " (Make sure the port supports WebSockets/WSS)";
    }
    chrome.storage.local.set({
      mqtt_status: "error",
      mqtt_error: msg,
    });
  });
}

// Periodic status sync
setInterval(() => {
  if (mqttClient) {
    const actualStatus = mqttClient.connected
      ? "connected"
      : mqttClient.reconnecting
        ? "connecting"
        : "disconnected";
    chrome.storage.local.get(["mqtt_status"], (res) => {
      if (res.mqtt_status !== actualStatus) {
        chrome.storage.local.set({ mqtt_status: actualStatus });
      }
    });
  }
}, 5000);

// Keep Service Worker alive and check connection
chrome.alarms.create("mqtt-keepalive", { periodInMinutes: 0.5 });
chrome.alarms.onAlarm.addListener((alarm) => {
  if (alarm.name === "mqtt-keepalive") {
    // console.log("Heartbeat: Checking MQTT connection...");
    chrome.storage.local.get(
      ["mqtt_host", "mqtt_port", "mqtt_enabled", "mqtt_is_editing"],
      (settings) => {
        if (
          settings.mqtt_host &&
          settings.mqtt_port &&
          settings.mqtt_enabled !== false &&
          !settings.mqtt_is_editing
        ) {
          if (!mqttClient) {
            console.log("Heartbeat: MQTT client missing, connecting...");
            connectMQTT();
          } else if (!mqttClient.connected && !mqttClient.reconnecting) {
            console.log(
              "Heartbeat: MQTT disconnected and not retrying, reconnecting...",
            );
            connectMQTT();
          }
        }
      },
    );
  }
});

// Initial connection - only if settings are complete
chrome.storage.local.get(
  ["mqtt_host", "mqtt_port", "mqtt_enabled", "mqtt_is_editing"],
  (settings) => {
    if (
      settings.mqtt_host &&
      settings.mqtt_port &&
      settings.mqtt_enabled !== false &&
      !settings.mqtt_is_editing
    ) {
      connectMQTT();
    } else {
      chrome.storage.local.set({ mqtt_status: "disconnected" });
    }
  },
);

// Reconnect when settings change (manual CONNECT button sets mqtt_reconnect)
chrome.storage.onChanged.addListener((changes) => {
  if (changes.mqtt_reconnect) {
    console.log("MQTT manual connect requested...");
    chrome.storage.local.get(
      ["mqtt_host", "mqtt_port", "mqtt_user", "mqtt_pass", "mqtt_enabled"],
      (settings) => {
        _doConnect(settings);
      },
    );
  }
});

chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
  if (request.action === "push-qr") {
    const { url } = request;

    chrome.storage.local.get(["esp_user", "esp_pass"], (res) => {
      const headers = {};
      if (res.esp_user || res.esp_pass) {
        headers["Authorization"] =
          "Basic " + btoa((res.esp_user || "") + ":" + (res.esp_pass || ""));
      }

      fetch(url, { method: "GET", headers: headers })
        .then((response) => {
          if (response.ok) {
            sendResponse({ success: true });
          } else {
            sendResponse({ success: false, status: response.status });
          }
        })
        .catch((error) => {
          sendResponse({ success: false, error: error.message });
        });
    });

    return true; // Keep message channel open for async response
  }
});

// --- Offscreen Document for Audio Playback ---
let creatingOffscreen;
async function setupOffscreenDocument(path) {
  const offscreenUrl = chrome.runtime.getURL(path);

  // Check if offscreen document already exists
  if (chrome.runtime.getContexts) {
    const existingContexts = await chrome.runtime.getContexts({
      contextTypes: ["OFFSCREEN_DOCUMENT"],
      documentUrls: [offscreenUrl],
    });

    if (existingContexts.length > 0) {
      return;
    }
  }

  // Handle race conditions
  if (creatingOffscreen) {
    await creatingOffscreen;
  } else {
    creatingOffscreen = chrome.offscreen.createDocument({
      url: path,
      reasons: ["AUDIO_PLAYBACK"],
      justification: "Play notification sound when receiving MQTT messages",
    });
    await creatingOffscreen;
    creatingOffscreen = null;
  }
}

async function playNotificationSound(data) {
  try {
    await setupOffscreenDocument("offscreen.html");
    chrome.runtime.sendMessage({
      action: "play-notification-sound",
      data: data,
    });
  } catch (err) {
    console.error("Error playing notification sound:", err);
  }
}
