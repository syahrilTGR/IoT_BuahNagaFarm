/*
 * Test: WiFi Manager + LED Builtin + Firebase RTDB
 *
 * Hardware minimal:
 *   - ESP32 DevKit (builtin LED di GPIO2)
 *   - Kabel jumper saja
 *   - WiFi / hotspot HP untuk config portal
 */

#include <Arduino.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "config.h"

// ── Firebase objects ──────────────────────────────────────────────────────────────
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ── WiFiManager object ───────────────────────────────────────────────────────
WiFiManager wm;

// ── Pin & State ──────────────────────────────────────────────────────────────
// ESP32 38-pin DevKit:
// - GPIO0 (Boot): HANYA untuk upload mode / serial. Jangan dipakai input!
// - GPIO2: Builtin LED (active LOW)
// - GPIO4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33: Safe GPIO untuk test
static const uint8_t LED_BUILTIN = 2;      // ESP32 builtin LED (GPIO2, active LOW)
static const uint8_t TEST_BTN_PIN = 4;     // GPIO4 - aman untuk tombol input

bool lampState      = false;
bool firebaseReady  = false;
bool wifiReady      = false;

// ── Timers (non-blocking millis) ─────────────────────────────────────────────
struct Timer {
  unsigned long prev = 0;
  unsigned long interval = 0;
  Timer() = default;
  Timer(unsigned long p, unsigned long i) : prev(p), interval(i) {}
};

Timer timerStatusSend{0, 5000};   // Kirim status ke Firebase tiap 5 detik
Timer timerCmdCheck {0, 200};     // Cek perintah dari Firebase tiap 200ms

// ── Forward declarations ─────────────────────────────────────────────────────
void setupWiFi();
void setupFirebase();
void sendStatusToFirebase();
void checkFirebaseCommand();
void setVirtualLamp(bool state);

// ════════════════════════════════════════════════════════════════════════════
// SETUP
// ════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== SmartNaga Test: WiFi + Firebase + LED ===");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // OFF (active LOW)
  pinMode(TEST_BTN_PIN, INPUT_PULLUP);  // GPIO4, aman untuk tombol (LOW when pressed)

  setupWiFi();
  setupFirebase();

  Serial.println("=== Setup Complete ===\n");
}

// ════════════════════════════════════════════════════════════════════════════
// LOOP (non-blocking)
// ════════════════════════════════════════════════════════════════════════════
void loop() {
  unsigned long now = millis();

  // Read physical button (GPIO4 with pullup -> LOW when pressed)
  static bool lastBtnState = HIGH;
  bool btnState = digitalRead(TEST_BTN_PIN);
  if (btnState != lastBtnState) {
    if (btnState == LOW) {  // Button pressed (connected to GND)
      setVirtualLamp(!lampState);  // Toggle lamp
      Serial.println("[BTN] Physical button pressed - toggling lamp");
    }
    lastBtnState = btnState;
  }

  if (now - timerCmdCheck.prev >= timerCmdCheck.interval) {
    timerCmdCheck.prev = now;
    checkFirebaseCommand();
  }

  if (now - timerStatusSend.prev >= timerStatusSend.interval) {
    timerStatusSend.prev = now;
    sendStatusToFirebase();
  }

  if (WiFi.status() != WL_CONNECTED) {
    wifiReady = false;
    Serial.println("[WiFi] Disconnected!");
  } else if (!wifiReady) {
    wifiReady = true;
    Serial.printf("[WiFi] Reconnected! IP: %s\n", WiFi.localIP().toString().c_str());
  }

  delay(10);   // Small delay untuk stabil
}

// ════════════════════════════════════════════════════════════════════════════
// WiFi Manager
// ════════════════════════════════════════════════════════════════════════════
void setupWiFi() {
  wm.setConfigPortalTimeout(180);  // 3 menit
  wm.setConnectTimeout(30);        // 30 detik

  String apName = "SmartNaga-Test-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  if (!wm.autoConnect(apName.c_str())) {
    Serial.println("[WiFi] Failed! Restarting...");
    delay(3000);
    ESP.restart();
  }

  wifiReady = true;
  Serial.println("[WiFi] Connected (WiFiManager)");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ════════════════════════════════════════════════════════════════════════════
// Firebase Setup
// ════════════════════════════════════════════════════════════════════════════
void setupFirebase() {
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_DATABASE_URL;

  auth.user.email = FIREBASE_USER_EMAIL;
  auth.user.password = FIREBASE_USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  delay(2000);

  if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
    firebaseReady = true;
    Serial.println("[Firebase] Connected & Authenticated");
  } else {
    firebaseReady = false;
    Serial.println("[Firebase] FAILED! Check credentials.");
  }
}

// ════════════════════════════════════════════════════════════════════════════
// Virtual Lamp Control (pakai LED builtin sebagai proxy)
// ════════════════════════════════════════════════════════════════════════════
void setVirtualLamp(bool state) {
  lampState = state;
  digitalWrite(LED_BUILTIN, state ? LOW : HIGH);  // active LOW
  Serial.printf("[LAMP] %s (LED %s)\n",
                state ? "ON" : "OFF",
                state ? "ON (LOW)" : "OFF (HIGH)");
}

// ════════════════════════════════════════════════════════════════════════════
// Firebase: Kirim status ke RTDB
// ════════════════════════════════════════════════════════════════════════════
void sendStatusToFirebase() {
  if (!wifiReady || !firebaseReady) return;

  FirebaseJson json;
  json.add("lamp", lampState);
  json.add("uptime", millis() / 1000);
  json.add("rssi", WiFi.RSSI());

  String path = "/test/status";
  if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
    Serial.println("[Firebase] Status sent to /test/status");
  } else {
    Serial.printf("[Firebase] Send FAILED (%s): %s\n",
                  path.c_str(), fbdo.errorReason().c_str());
  }
}

// ════════════════════════════════════════════════════════════════════════════
// Firebase: Baca perintah dari RTDB
// ════════════════════════════════════════════════════════════════════════════
void checkFirebaseCommand() {
  if (!wifiReady || !firebaseReady) return;

  if (Firebase.RTDB.getBool(&fbdo, "/test/control")) {
    bool cmd = fbdo.boolData();
    if (cmd != lampState) {
      setVirtualLamp(cmd);
      Serial.println("[Firebase] Command executed.");
    }
  }
}
