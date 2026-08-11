#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <WiFiManager.h>  // ESP32 WiFi Manager
#include <Firebase_ESP_Client.h>
#include "config.h"

// NTP time sync
#include "time.h"

// ── Pin Definition ───────────────────────────────────────────────────────────
static const uint8_t SSR_PIN = 13;          // D13 → LLC → SSR (Blok A)
static const uint8_t SSR_PIN_B = 27;        // GPIO27 → SSR (Blok B)
static const uint8_t PZEM_RX = 33;          // ESP32 RX2 ← PZEM TX
static const uint8_t PZEM_TX = 32;          // ESP32 TX2 → PZEM RX
static const uint8_t PZEMB_RX = 25;         // I2C SDA
static const uint8_t PZEMB_TX = 26;         // I2C SCL
static const uint8_t TEST_BTN_PIN = 4;      // GPIO4 - tombol toggle lampu (INPUT_PULLUP)

// ── PZEM Objects ─────────────────────────────────────────────────────────────
// PZEM004Tv30 constructor akan panggil Serial2.begin() otomatis
PZEM004Tv30 pzemA(Serial2, PZEM_RX, PZEM_TX);  // Blok A, addr default 0xF8
PZEM004Tv30 pzemB(Serial1, PZEMB_RX, PZEMB_TX); // Blok B, addr default 0xF9

// ── Timers (non-blocking via millis) ─────────────────────────────────────────
struct Timer {
  unsigned long prev = 0;
  unsigned long interval = 0;
  Timer() = default;
  Timer(unsigned long p, unsigned long i) : prev(p), interval(i) {}
};

Timer timerPZEM      {0, 5000};   // Baca PZEM tiap 5 detik
Timer timerFirebase  {0, 10000};  // Kirim ke Firebase tiap 10 detik
Timer timerOLED      {0, 1000};   // Update OLED tiap 1 detik
Timer timerCheckCmd  {0, 100};    // Cek perintah Firebase tiap 100ms

// ── State ────────────────────────────────────────────────────────────────────
struct EnergyData {
  float voltage = 0;
  float current = 0;
  float power = 0;
  float energy = 0;
  bool valid = false;
};

EnergyData blockA;
EnergyData blockB;
bool lampAState = false;
bool lampBState = false;
bool wifiConnected = false;
bool firebaseReady = false;  // Firebase connection status

// Debounce: ignore Firebase commands within 2s after last execution
unsigned long lastCmdExecTime = 0;
const unsigned long CMD_DEBOUNCE_MS = 2000;

// ── Firebase objects ──────────────────────────────────────────────────────────────
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ── WiFiManager Instance ─────────────────────────────────────────────────────
WiFiManager wm;

// ── Forward Declarations ─────────────────────────────────────────────────────
void setupWiFi();
void setupFirebase();
void readPZEM();
void controlSSR();
void controlSSRB();
void setLampA(bool state);
void setLampB(bool state);
void updateOLED();
void checkFirebaseCommands();
void sendToFirebase();
void printEnergyData(const EnergyData& d, const char* label);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== SmartNaga ESP32 Boot ===");

  // SSR pin
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);  // Default OFF
  pinMode(SSR_PIN_B, OUTPUT);
  digitalWrite(SSR_PIN_B, LOW);

  // Tombol toggle lampu (INPUT_PULLUP, tombol ke GND)
  pinMode(TEST_BTN_PIN, INPUT_PULLUP);

  // LED builtin sebagai indikator (active LOW)
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  // Serial2 sudah di-init otomatis oleh PZEM004Tv30 constructor
  // delay(200);  // beri waktu UART stabil

  // WiFi Manager (auto-connect atau AP config portal)
  setupWiFi();
  setupFirebase();

  Serial.println("=== Setup Selesai ===\n");
}

void loop() {
  unsigned long now = millis();

  // Read physical button (GPIO4 with pullup - LOW when pressed)
  // Debounce: ignore multiple triggers dalam 200ms
  static unsigned long lastBtnTime = 0;
  static bool lastBtnState = HIGH;
  const unsigned long debounceMs = 200;  // 200ms debounce interval

  bool btnState = digitalRead(TEST_BTN_PIN);
  if (btnState != lastBtnState) {
    if (btnState == LOW && (now - lastBtnTime >= debounceMs)) {  // Tombol ditekan (disambungkan ke GND)
      lastBtnTime = now;
      setLampA(!lampAState);  // Toggle lampu
      Serial.println("[BTN] Physical button pressed - toggling lamp");
    }
    lastBtnState = btnState;
  }

  // 1. Baca PZEM tiap 5 detik
  if (now - timerPZEM.prev >= timerPZEM.interval) {
    timerPZEM.prev = now;
    readPZEM();
  }

  // 2. Kontrol SSR (real-time)
  controlSSR();

  // 3. Cek perintah dari Firebase tiap 100ms
  if (now - timerCheckCmd.prev >= timerCheckCmd.interval) {
    timerCheckCmd.prev = now;
    checkFirebaseCommands();
  }

  // 4. Kirim data ke Firebase tiap 10 detik
  if (now - timerFirebase.prev >= timerFirebase.interval) {
    timerFirebase.prev = now;
    sendToFirebase();
  }

  // 5. Update OLED tiap 1 detik
  if (now - timerOLED.prev >= timerOLED.interval) {
    timerOLED.prev = now;
    updateOLED();
  }

  // Watchdog / keep WiFi alive
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
  } else if (!wifiConnected) {
    wifiConnected = true;
    Serial.println("WiFi Reconnected");
  }
}

// ════════════════════════════════════════════════════════════════════════════
// WiFi Manager Setup
// ════════════════════════════════════════════════════════════════════════════
void setupWiFi() {
  wm.setConfigPortalTimeout(180);  // 3 menit timeout config portal
  wm.setConnectTimeout(30);        // 30 detik timeout connect

  // Custom AP name
  String apName = String("SmartNaga-") + String((uint32_t)ESP.getEfuseMac(), HEX);

  bool res = wm.autoConnect(apName.c_str());

  if (!res) {
    Serial.println("WiFi Connect Failed! Restarting...");
    delay(3000);
    ESP.restart();
  }

  wifiConnected = true;
  Serial.print("WiFi Connected! IP: ");
  Serial.println(WiFi.localIP());

  // Sync waktu via NTP
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("[NTP] Waiting for time sync...");
  time_t now = time(nullptr);
  while (now < 1000000000) {
    delay(500);
    now = time(nullptr);
  }
  Serial.printf("[NTP] Time synced: %s", ctime(&now));
}

// Helper: get epoch seconds (fallback millis()/1000 if NTP not ready)
unsigned long getEpochSec() {
  time_t t = time(nullptr);
  if (t > 1000000000) return (unsigned long)t;
  return millis() / 1000;
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

    // Inisialisasi default control paths
    Firebase.RTDB.setBool(&fbdo, "/lamp/control/a", false);
    Firebase.RTDB.setBool(&fbdo, "/lamp/control/b", false);
    Serial.println("[Firebase] Default control paths initialized.");

    // Sync awal: baca status lampu dari RTDB
    bool stateA = false;
    if (Firebase.RTDB.getBool(&fbdo, "/lamp/control/a")) {
      stateA = fbdo.boolData();
    }
    setLampA(stateA);

    bool stateB = false;
    if (Firebase.RTDB.getBool(&fbdo, "/lamp/control/b")) {
      stateB = fbdo.boolData();
    }
    setLampB(stateB);
    Serial.println("[Firebase] Initial lamp state synced from RTDB.");
  } else {
    firebaseReady = false;
    Serial.println("[Firebase] FAILED! Check credentials.");
  }
}

// ════════════════════════════════════════════════════════════════════════════
// PZEM Reading
// ════════════════════════════════════════════════════════════════════════════
void readPZEM() {
  // Cek alamat PZEM (debug)
  static bool addrPrinted = false;
  if (!addrPrinted) {
    uint8_t addr = pzemA.readAddress();
    Serial.printf("[PZEM] Address: 0x%02X\n", addr);
    addrPrinted = true;
  }

  // Cek alamat PZEM B
  static bool addrBPrinted = false;
  if (!addrBPrinted) {
    uint8_t addrB = pzemB.readAddress();
    if (addrB == 0 || addrB == 0xFF) {
      Serial.println("[PZEM-B] NO RESPONSE - check wiring!");
    } else {
      Serial.printf("[PZEM-B] Address: 0x%02X\n", addrB);
    }
    addrBPrinted = true;
  }

  // Baca data Blok B
  blockB.voltage = pzemB.voltage();
  blockB.current = pzemB.current();
  blockB.power   = pzemB.power();
  blockB.energy  = pzemB.energy();
  blockB.valid = !isnan(blockB.voltage) && !isnan(blockB.current);

  if (blockB.valid) {
    printEnergyData(blockB, "Blok B");
  } else {
    Serial.println("[PZEM-B] No valid data");
  }

  blockA.voltage = pzemA.voltage();
  blockA.current = pzemA.current();
  blockA.power   = pzemA.power();
  blockA.energy  = pzemA.energy();

  // Validasi per-field (seperti contoh mandulaj)
  blockA.valid = !isnan(blockA.voltage) && !isnan(blockA.current);

  if (blockA.valid) {
    printEnergyData(blockA, "Blok A");
  } else {
    Serial.println("[PZEM] Error: Gagal baca data Blok A");
  }
}

void printEnergyData(const EnergyData& d, const char* label) {
  Serial.printf("[%s] V: %.1fV  I: %.3fA  P: %.1fW  E: %.3fkWh\n",
                label, d.voltage, d.current, d.power, d.energy);
}

// ════════════════════════════════════════════════════════════════════════════
// SSR Control
// ════════════════════════════════════════════════════════════════════════════
void controlSSR() {
  // Active LOW: LOW = ON, HIGH = OFF (SSR direct dari ESP32 tanpa LLC)
  digitalWrite(SSR_PIN, !lampAState);
}

void controlSSRB() {
  digitalWrite(SSR_PIN_B, !lampBState);
}

void setLampA(bool state) {
  lampAState = state;
  controlSSR();
  Serial.printf("[SSR] Lampu Blok A: %s\n", state ? "ON" : "OFF");
}

void setLampB(bool state) {
  lampBState = state;
  controlSSRB();
  Serial.printf("[SSR] Lampu Blok B: %s\n", state ? "ON" : "OFF");
}

// ════════════════════════════════════════════════════════════════════════════
// OLED Display (placeholder - implement nanti)
// ════════════════════════════════════════════════════════════════════════════
void updateOLED() {
  // TODO: Implement OLED I2C (SSD1306) di sini
  // Contoh nanti:
  // display.clearDisplay();
  // display.setCursor(0,0);
  // display.printf("Blok A: %.1fV %.2fA", blockA.voltage, blockA.current);
  // display.display();
}

// ════════════════════════════════════════════════════════════════════════════
// Firebase Integration
// ════════════════════════════════════════════════════════════════════════════
void checkFirebaseCommands() {
  if (!wifiConnected || !firebaseReady) return;

  unsigned long now = millis();

  // Baca /lamp/control/a (boolean)
  if (Firebase.RTDB.getBool(&fbdo, "/lamp/control/a")) {
    bool cmd = fbdo.boolData();
    if (cmd != lampAState && (now - lastCmdExecTime >= CMD_DEBOUNCE_MS)) {
      setLampA(cmd);
      lastCmdExecTime = now;
      Serial.println("[Firebase] Command executed Blok A.");
    }
  }

  // Baca /lamp/control/b (boolean)
  if (Firebase.RTDB.getBool(&fbdo, "/lamp/control/b")) {
    bool cmd = fbdo.boolData();
    if (cmd != lampBState && (now - lastCmdExecTime >= CMD_DEBOUNCE_MS)) {
      setLampB(cmd);
      lastCmdExecTime = now;
      Serial.println("[Firebase] Command executed Blok B.");
    }
  } else {
    // Debug: tampilkan error jika path belum ada
    static bool loggedB = false;
    if (!loggedB && fbdo.errorReason().length() > 0) {
      Serial.printf("[Firebase] Blok B read error: %s\n", fbdo.errorReason().c_str());
      loggedB = true;
    }
  }
}

void sendToFirebase() {
  if (!wifiConnected || !firebaseReady) return;

  unsigned long ts = getEpochSec();

  // Kirim energy data ke /energy/block_a
  FirebaseJson energyJson;
  energyJson.set("voltage", blockA.voltage);
  energyJson.set("current", blockA.current);
  energyJson.set("power", blockA.power);
  energyJson.set("energy", blockA.energy);
  energyJson.set("timestamp", (double)ts);
  Firebase.RTDB.setJSON(&fbdo, "/energy/block_a", &energyJson);

  // Kirim energy data ke /energy/block_b
  FirebaseJson energyJsonB;
  energyJsonB.set("voltage", blockB.voltage);
  energyJsonB.set("current", blockB.current);
  energyJsonB.set("power", blockB.power);
  energyJsonB.set("energy", blockB.energy);
  energyJsonB.set("timestamp", (double)ts);
  Firebase.RTDB.setJSON(&fbdo, "/energy/block_b", &energyJsonB);

  // Note: /lamp/status/{a,b} ditulis oleh aplikasi (updateLampControl)
  // Firmware hanya baca /lamp/control/{a,b} untuk eksekusi perintah
}

// ════════════════════════════════════════════════════════════════════════════
// Utility
// ════════════════════════════════════════════════════════════════════════════
void printResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset Reason: %d\n", reason);
}