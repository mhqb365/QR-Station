document.addEventListener("DOMContentLoaded", () => {
  const ipInput = document.getElementById("ip");
  const targetAccSelect = document.getElementById("targetAccount");
  const amtInput = document.getElementById("amt");
  const pushBtn = document.getElementById("push");
  const searchBtn = document.getElementById("search");
  const statusDiv = document.getElementById("status");
  const espUserInput = document.getElementById("esp_user");
  const espPassInput = document.getElementById("esp_pass");

  // Tab Logic
  const tabs = document.querySelectorAll(".tab");
  const tabContents = document.querySelectorAll(".tab-content");

  const BANK_MAP = {
    970436: "Vietcombank",
    970415: "VietinBank",
    970418: "BIDV",
    970405: "Agribank",
    970422: "MB Bank",
    970407: "Techcombank",
    970423: "TPBank",
    970432: "VPBank",
    970403: "Sacombank",
    970441: "VIB",
    970443: "SHB",
    970428: "NAM A BANK",
    970437: "HDBank",
    970454: "VietCapitalBank",
    970429: "SCB",
    970452: "KienLongBank",
    970430: "PGBank",
    970448: "OCB",
    970431: "Eximbank",
    970425: "ABBANK",
    970416: "ACB",
    970427: "VietABank",
    970440: "SEABank",
    970419: "NCB",
    970438: "BaoVietBank",
    970406: "DongA Bank",
    970433: "VietBank",
    970424: "Shinhan Bank",
    970412: "PVcomBank",
    970400: "SaigonBank",
    970449: "LPBank",
    970414: "OceanBank",
    970439: "Public Bank",
    970455: "Woori Bank",
    970408: "GPBank",
    970457: "Wooribank",
    970426: "MSB",
  };

  tabs.forEach((tab) => {
    tab.addEventListener("click", () => {
      tabs.forEach((t) => t.classList.remove("active"));
      tabContents.forEach((c) => c.classList.remove("active"));

      tab.classList.add("active");
      const contentId = `tab-${tab.dataset.tab}`;
      document.getElementById(contentId).classList.add("active");
    });
  });

  let localAccounts = [];

  // Load saved settings
  chrome.storage.local.get(
    [
      "esp_ip",
      "esp_user",
      "esp_pass",
      "last_acc_idx",
      "mqtt_host",
      "mqtt_port",
      "mqtt_user",
      "mqtt_pass",
      "mqtt_enabled",
      "mqtt_status",
      "mqtt_error",
      "qr_default_content",
      "mqtt_sound",
      "local_accounts",
    ],
    (res) => {
      console.log("Popup loaded, MQTT status:", res.mqtt_status);
      if (res.esp_ip) ipInput.value = res.esp_ip;
      if (res.esp_user) espUserInput.value = res.esp_user;
      if (res.esp_pass) espPassInput.value = res.esp_pass;

      if (res.mqtt_host)
        document.getElementById("mqtt_host").value = res.mqtt_host;
      if (res.mqtt_port)
        document.getElementById("mqtt_port").value = res.mqtt_port;
      if (res.mqtt_user)
        document.getElementById("mqtt_user").value = res.mqtt_user;
      if (res.mqtt_pass)
        document.getElementById("mqtt_pass").value = res.mqtt_pass;

      const mqttEnabled = document.getElementById("mqtt_enabled");
      const mqttSound = document.getElementById("mqtt_sound");

      mqttEnabled.checked = res.mqtt_enabled !== false;
      toggleMqttFields(mqttEnabled.checked);
      updateMqttStatusDisplay(res.mqtt_status, res.mqtt_error);
      updateMqttSoundState(mqttEnabled.checked);

      if (res.local_accounts) {
        localAccounts = res.local_accounts;
      }
      renderAccountSelect(res.last_acc_idx);
      renderAccountList();

      const qrContentInput = document.getElementById("qr_default_content");
      if (res.qr_default_content !== undefined) {
        qrContentInput.value = res.qr_default_content;
      }
      qrContentInput.addEventListener("input", (e) => {
        chrome.storage.local.set({ qr_default_content: e.target.value });
      });

      if (mqttSound) {
        mqttSound.checked = res.mqtt_sound !== false;
        mqttSound.addEventListener("change", (e) => {
          chrome.storage.local.set({ mqtt_sound: e.target.checked });
        });
      }

      validateEspInputs();
    },
  );

  function validateEspInputs() {
    const user = espUserInput.value.trim();
    const pass = espPassInput.value.trim();
    searchBtn.disabled = !user || !pass;
    searchBtn.style.opacity = searchBtn.disabled ? "0.5" : "1";
    searchBtn.style.cursor = searchBtn.disabled ? "not-allowed" : "pointer";
  }

  [espUserInput, espPassInput].forEach((el) => {
    el.addEventListener("input", validateEspInputs);
  });

  function updateMqttSoundState(enabled) {
    const mqttSound = document.getElementById("mqtt_sound");
    if (!mqttSound) return;
    mqttSound.disabled = !enabled;
    const slider = mqttSound.parentElement.querySelector(".slider");
    if (slider) {
      slider.style.opacity = enabled ? "1" : "0.5";
      slider.style.cursor = enabled ? "pointer" : "not-allowed";
    }
  }

  let isMqttSaving = false;

  function updateMqttStatusDisplay(status, error, attempt) {
    const indicators = document.querySelectorAll(".mqtt-status-indicator");
    const mqttInfo = document.getElementById("mqtt_info");
    if (indicators.length === 0) return;

    indicators.forEach((indicator) => {
      indicator.className =
        "status-dot " + (status || "disconnected") + " mqtt-status-indicator";
      indicator.title = status
        ? status.charAt(0).toUpperCase() + status.slice(1)
        : "Disconnected";
    });

    const mqttSaveBtn = document.getElementById("mqtt_save");

    if (mqttSaveBtn) {
      const isConnecting = status === "connecting";
      mqttSaveBtn.disabled = isConnecting;
      mqttSaveBtn.style.opacity = isConnecting ? "0.5" : "1";
      mqttSaveBtn.style.cursor = isConnecting ? "not-allowed" : "pointer";
      // Change text to reflect state if desired, or keep as is.
    }

    if (mqttInfo) {
      if (status === "connected") {
        showStatus("✅ Connected successfully", "success", "mqtt_info");
        isMqttSaving = false;
      } else if (status === "connecting") {
        const curAttempt = attempt || 1;
        showStatus(
          `⏳ Connecting (Attempt ${curAttempt}/3)...`,
          "info",
          "mqtt_info",
        );
      } else if (status === "error") {
        showStatus(
          "❌ Error: " + (error || "Connection failed"),
          "error",
          "mqtt_info",
        );
        isMqttSaving = false;
      } else if (status === "disconnected") {
        if (
          !mqttInfo.textContent.includes("❌ Error:") &&
          !mqttInfo.classList.contains("success")
        ) {
          mqttInfo.style.display = "none";
        }
      }
    }
  }

  chrome.storage.onChanged.addListener((changes) => {
    if (changes.mqtt_status || changes.mqtt_attempt || changes.mqtt_error) {
      chrome.storage.local.get(
        ["mqtt_status", "mqtt_error", "mqtt_attempt"],
        (res) => {
          updateMqttStatusDisplay(
            res.mqtt_status,
            res.mqtt_error,
            res.mqtt_attempt,
          );
        },
      );
    }
    if (changes.local_accounts) {
      localAccounts = changes.local_accounts.newValue || [];
      renderAccountSelect();
      renderAccountList();
    }
  });

  function toggleMqttFields(enabled) {
    document.getElementById("mqtt_fields").style.opacity = enabled
      ? "1"
      : "0.5";
    document.getElementById("mqtt_fields").style.pointerEvents = enabled
      ? "auto"
      : "none";
  }

  // --- MQTT EDITING STATE ---
  const mqttInputs = ["mqtt_host", "mqtt_port", "mqtt_user", "mqtt_pass"].map(
    (id) => document.getElementById(id),
  );

  mqttInputs.forEach((el) => {
    if (!el) return;
    el.addEventListener("focus", () => {
      chrome.storage.local.set({ mqtt_is_editing: true });
    });
    // We don't clear on blur because user might be switching between host/port
  });

  // Clear editing state when popup is closed or hidden
  window.addEventListener("unload", () => {
    chrome.storage.local.set({ mqtt_is_editing: false });
  });

  // Clean MQTT settings
  document.getElementById("mqtt-clean")?.addEventListener("click", () => {
    if (confirm("Clear all MQTT settings?")) {
      document.getElementById("mqtt_host").value = "";
      document.getElementById("mqtt_port").value = "";
      document.getElementById("mqtt_user").value = "";
      document.getElementById("mqtt_pass").value = "";

      chrome.storage.local.set({
        mqtt_host: "",
        mqtt_port: "",
        mqtt_user: "",
        mqtt_pass: "",
        mqtt_reconnect: Date.now(),
        mqtt_status: "disconnected",
      });

      showStatus("MQTT settings cleared", "success", "mqtt_info");
    }
  });

  // Clean Device settings
  document.getElementById("device-clean")?.addEventListener("click", () => {
    if (confirm("Clear all Device settings?")) {
      document.getElementById("ip").value = "";
      document.getElementById("esp_user").value = "";
      document.getElementById("esp_pass").value = "";

      chrome.storage.local.set({
        esp_ip: "",
        esp_user: "",
        esp_pass: "",
      });

      showStatus("Device settings cleared", "success", "device_info");
    }
  });

  document.getElementById("mqtt_save").addEventListener("click", () => {
    const host = document.getElementById("mqtt_host").value.trim();
    const port = document.getElementById("mqtt_port").value.trim();
    if (!host || !port) return;

    isMqttSaving = true;
    showStatus("⏳ Connecting...", "info", "mqtt_info");

    // Clear editing state before manual connect
    chrome.storage.local.set({
      mqtt_is_editing: false,
      mqtt_host: host,
      mqtt_port: port,
      mqtt_user: document.getElementById("mqtt_user").value.trim(),
      mqtt_pass: document.getElementById("mqtt_pass").value.trim(),
      mqtt_reconnect: Date.now(),
    });
  });

  document.getElementById("mqtt_enabled").addEventListener("change", (e) => {
    chrome.storage.local.set({ mqtt_enabled: e.target.checked });
    toggleMqttFields(e.target.checked);
    updateMqttSoundState(e.target.checked);
  });

  // --- ACCOUNT MANAGEMENT ---
  const accountModal = document.getElementById("account-modal");
  const accountListDiv = document.getElementById("account-list");
  const addAccountBtn = document.getElementById("add-account");
  const modalCancelBtn = document.getElementById("modal-cancel");
  const modalSaveBtn = document.getElementById("modal-save");

  function renderAccountList() {
    accountListDiv.innerHTML = "";
    if (localAccounts.length === 0) {
      accountListDiv.innerHTML =
        '<div style="text-align:center;color:#666;font-size:0.8rem;padding:10px;">No account found</div>';
      return;
    }

    localAccounts.forEach((acc, idx) => {
      const item = document.createElement("div");
      item.className = "account-item";
      const bankName = BANK_MAP[acc.bin] || `BIN: ${acc.bin}`;
      item.innerHTML = `
        <div class="account-info">
          <div class="name">${acc.name}</div>
          <div class="details">${acc.acc} - ${bankName}</div>
        </div>
        <div class="account-actions">
          <button class="btn-small edit-btn" data-idx="${idx}">✏️</button>
          <button class="btn-small delete-btn" data-idx="${idx}" style="color:#ff4444;">🗑️</button>
        </div>
      `;
      accountListDiv.appendChild(item);
    });

    document.querySelectorAll(".edit-btn").forEach((btn) => {
      btn.addEventListener("click", () => openAccountModal(btn.dataset.idx));
    });

    document.querySelectorAll(".delete-btn").forEach((btn) => {
      btn.addEventListener("click", () => {
        if (confirm("Delete this account?")) {
          localAccounts.splice(btn.dataset.idx, 1);
          chrome.storage.local.set({ local_accounts: localAccounts });
        }
      });
    });
  }

  function renderAccountSelect(savedIdx = null) {
    targetAccSelect.innerHTML = "";
    if (localAccounts.length === 0) {
      targetAccSelect.innerHTML =
        '<option value="">-- No account found --</option>';
      return;
    }

    localAccounts.forEach((acc, index) => {
      const opt = document.createElement("option");
      opt.value = index;
      opt.textContent = `${acc.name} - ${acc.acc}`;
      targetAccSelect.appendChild(opt);
    });

    if (savedIdx !== null && localAccounts[savedIdx]) {
      targetAccSelect.value = savedIdx;
    } else {
      updateSavedAccount();
    }
  }

  function openAccountModal(idx = null) {
    const title = document.getElementById("modal-title");
    const editIdx = document.getElementById("edit-idx");
    const accName = document.getElementById("acc-name");
    const accNum = document.getElementById("acc-num");
    const accBin = document.getElementById("acc-bin");

    if (idx !== null) {
      title.textContent = "Edit account";
      editIdx.value = idx;
      accName.value = localAccounts[idx].name;
      accNum.value = localAccounts[idx].acc;
      accBin.value = localAccounts[idx].bin;
    } else {
      title.textContent = "Add account";
      editIdx.value = "";
      accName.value = "";
      accNum.value = "";
      accBin.value = "";
    }
    accountModal.classList.add("active");
  }

  addAccountBtn.addEventListener("click", () => openAccountModal());
  modalCancelBtn.addEventListener("click", () =>
    accountModal.classList.remove("active"),
  );

  modalSaveBtn.addEventListener("click", () => {
    const idx = document.getElementById("edit-idx").value;
    const name = document.getElementById("acc-name").value.trim();
    const acc = document.getElementById("acc-num").value.trim();
    const bin = document.getElementById("acc-bin").value.trim();

    if (!name || !acc || !bin) {
      alert("Please enter full information");
      return;
    }

    const newAcc = { name, acc, bin };
    if (idx !== "") {
      localAccounts[idx] = newAcc;
    } else {
      localAccounts.push(newAcc);
    }

    chrome.storage.local.set({ local_accounts: localAccounts });
    accountModal.classList.remove("active");
  });

  targetAccSelect.addEventListener("change", () => updateSavedAccount());

  function updateSavedAccount() {
    const idx = targetAccSelect.value;
    if (idx !== "" && localAccounts[idx]) {
      const acc = localAccounts[idx];
      chrome.storage.local.set({
        last_acc_idx: idx,
        last_bin: acc.bin,
        last_acc: acc.acc,
        last_owner: acc.name,
      });
    }
  }

  // --- IMPORT / EXPORT JSON ---
  const fileInput = document.getElementById("file-input");
  document
    .getElementById("import-json")
    .addEventListener("click", () => fileInput.click());

  fileInput.addEventListener("change", (e) => {
    const file = e.target.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (event) => {
      try {
        const data = JSON.parse(event.target.result);
        if (Array.isArray(data)) {
          // Support both 'name' and 'on' for owner name
          const mappedData = data
            .map((a) => ({
              name: a.name || a.on || "No name",
              acc: a.acc || "",
              bin: a.bin || "",
            }))
            .filter((a) => a.acc && a.bin);

          if (mappedData.length > 0) {
            localAccounts = [...localAccounts, ...mappedData];
            chrome.storage.local.set({ local_accounts: localAccounts });
            showStatus(
              "✅ Imported " + mappedData.length + " accounts",
              "success",
            );
          } else {
            alert(
              "File does not contain valid account data (requires acc and bin)",
            );
          }
        }
      } catch (err) {
        alert("Error reading JSON file");
      }
    };
    reader.readAsText(file);
  });

  document.getElementById("export-json").addEventListener("click", () => {
    if (localAccounts.length === 0) return;
    const blob = new Blob([JSON.stringify(localAccounts, null, 2)], {
      type: "application/json",
    });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = "qr_station_accounts.json";
    a.click();
    URL.revokeObjectURL(url);
  });

  // --- DEVICE SCAN & PUSH ---
  async function checkIp(ip) {
    try {
      const controller = new AbortController();
      const timeoutId = setTimeout(() => controller.abort(), 1500);
      const user = espUserInput.value.trim();
      const pass = espPassInput.value.trim();
      const headers = {};
      if (user || pass)
        headers["Authorization"] = "Basic " + btoa(user + ":" + pass);

      const response = await fetch(`http://${ip}/api/accounts`, {
        signal: controller.signal,
        headers,
      });

      if (response.status === 401) return 401; // Auth failed

      if (response.ok) {
        return 200; // Success and Authed
      }
    } catch (e) {}
    return 0; // Not found or error
  }

  searchBtn.addEventListener("click", async () => {
    showStatus("🔍 Searching device...", "info", "device_info");
    searchBtn.disabled = true;

    const currentIp = ipInput.value.trim();
    const initHosts = [];
    if (currentIp) initHosts.push(currentIp);
    initHosts.push("qrstation.local");

    for (const h of initHosts) {
      const status = await checkIp(h);
      if (status === 200) {
        ipInput.value = h;
        chrome.storage.local.set({
          esp_ip: h,
          esp_user: espUserInput.value.trim(),
          esp_pass: espPassInput.value.trim(),
        });
        showStatus("✅ Connected: " + h, "success", "device_info");
        searchBtn.disabled = false;
        validateEspInputs();
        return;
      } else if (status === 401) {
        showStatus("❌ Wrong User or Pass: " + h, "error", "device_info");
        searchBtn.disabled = false;
        validateEspInputs();
        return;
      }
    }

    showStatus("📡 Scanning network...", "info", "device_info");
    const subnets = ["192.168.1", "192.168.10", "192.168.100", "192.168.0"];
    const BATCH_SIZE = 50;
    let authFailedIp = null;

    for (const subnet of subnets) {
      for (let i = 1; i < 255; i += BATCH_SIZE) {
        const batch = [];
        for (let j = i; j < i + BATCH_SIZE && j < 255; j++)
          batch.push(`${subnet}.${j}`);

        const results = await Promise.all(
          batch.map(async (ip) => {
            const status = await checkIp(ip);
            return { ip, status };
          }),
        );

        const success = results.find((r) => r.status === 200);
        if (success) {
          ipInput.value = success.ip;
          chrome.storage.local.set({
            esp_ip: success.ip,
            esp_user: espUserInput.value.trim(),
            esp_pass: espPassInput.value.trim(),
          });
          showStatus("✅ Connected: " + success.ip, "success", "device_info");
          searchBtn.disabled = false;
          validateEspInputs();
          return;
        }

        const authFailed = results.find((r) => r.status === 401);
        if (authFailed) authFailedIp = authFailed.ip;
      }
    }

    if (authFailedIp) {
      showStatus(
        "❌ Wrong User or Pass: " + authFailedIp,
        "error",
        "device_info",
      );
    } else {
      showStatus("❌ Device not found", "error", "device_info");
    }
    searchBtn.disabled = false;
    validateEspInputs();
  });

  pushBtn.addEventListener("click", async () => {
    const ip = ipInput.value.trim();
    const idx = targetAccSelect.value;
    const amt = amtInput.value.trim();
    const desc = document.getElementById("qr_default_content").value.trim();

    if (idx === "") {
      showStatus("Please select account", "error");
      return;
    }

    const account = localAccounts[idx];
    showStatus("🚀 Creating QR...", "info");

    const qrData = {
      bin: account.bin,
      acc: account.acc,
      amount: amt,
      owner: account.name,
      desc: desc,
    };

    // Show in browser tabs (Content Script)
    chrome.tabs.query({ active: true, currentWindow: true }, (tabs) => {
      if (tabs.length > 0) {
        chrome.tabs.sendMessage(
          tabs[0].id,
          { action: "show-qr-modal", data: qrData },
          () => {
            if (chrome.runtime.lastError)
              console.log(
                "Content script error:",
                chrome.runtime.lastError.message,
              );
          },
        );
      }
    });

    // Push to hardware ONLY if IP is provided
    if (ip) {
      const url = `http://${ip}/api/qr?bin=${account.bin}&acc=${account.acc}&amt=${amt}&on=${encodeURIComponent(account.name)}&desc=${encodeURIComponent(desc)}`;
      const user = espUserInput.value.trim();
      const pass = espPassInput.value.trim();
      const headers = {};
      if (user || pass)
        headers["Authorization"] = "Basic " + btoa(user + ":" + pass);

      try {
        const controller = new AbortController();
        setTimeout(() => controller.abort(), 5000);
        const response = await fetch(url, {
          method: "GET",
          signal: controller.signal,
          headers,
        });
        if (response.ok) {
          showStatus("✅ Create QR successfully", "success");
        } else {
          showStatus(
            "⚠️ Create QR failed (Device response error: " +
              response.status +
              ")",
            "error",
          );
        }
      } catch (err) {
        showStatus("⚠️ Create QR failed (Cannot connect to device)", "info");
      }
    } else {
      showStatus("✅ Create QR on screen", "success");
    }
  });

  function showStatus(msg, type, targetId = "status") {
    const target =
      targetId === "status" ? statusDiv : document.getElementById(targetId);
    if (!target) return;
    target.textContent = msg;
    target.className = "status " + type;
    target.style.display = "block";
    if (type === "success") {
      setTimeout(() => {
        if (target.textContent === msg) target.style.display = "none";
      }, 5000);
    }
  }
});
