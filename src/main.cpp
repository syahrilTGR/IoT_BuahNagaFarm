#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <WiFiManager.h>  // ESP32 WiFi Manager
#include <Firebase_ESP_Client.h>
#include "config.h"

// ── Pin Definition ───────────────────────────────────────────────────────────
static const uint8_t SSR_PIN = 13;          // D13 → LLC → SSR (Blok A)
static const uint8_t SSR_PIN_B = 12;        // D12 → LLC → SSR (Blok B) - future
static const uint8_t PZEM_RX = 33;          // ESP32 RX2 ← PZEM TX
static const uint8_t PZEM_TX = 32;          // ESP32 TX2 → PZEM RX
static const uint8_t TEST_BTN_PIN = 4;      // GPIO4 - tombol toggle lampu (INPUT_PULLUP)

// ── PZEM Objects ─────────────────────────────────────────────────────────────
// PZEM004Tv30 constructor akan panggil Serial2.begin() otomatis
PZEM004Tv30 pzemA(Serial2, PZEM_RX, PZEM_TX);  // Blok A, addr default 0xF8
// PZEM004Tv30 pzemB(Serial1, ...);             // Blok B - future

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
bool lampAState = false;
bool wifiConnected = false;
bool firebaseReady = false;  // Firebase connection status

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
void setLampA(bool state);
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
  String apName = "SmartNaga-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  bool res = wm.autoConnect(apName.c_str());

  if (!res) {
    Serial.println("WiFi Connect Failed! Restarting...");
    delay(3000);
    ESP.restart();
  }

  wifiConnected = true;
  Serial.print("WiFi Connected! IP: ");
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
  // Active HIGH: HIGH = ON, LOW = OFF
  digitalWrite(SSR_PIN, lampAState ? HIGH : LOW);
  // LED builtin sebagai indikator (active LOW: ON = LOW)
  digitalWrite(2, lampAState ? LOW : HIGH);
}

void setLampA(bool state) {
  lampAState = state;
  controlSSR();
  Serial.printf("[SSR] Lampu Blok A: %s\n", state ? "ON" : "OFF");
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

  // Baca /lamp/control/a (boolean)
  if (Firebase.RTDB.getBool(&fbdo, "/lamp/control/a")) {
    bool cmd = fbdo.boolData();
    if (cmd != lampAState) {
      setLampA(cmd);
      Serial.println("[Firebase] Command executed.");
    }
  }
}

void sendToFirebase() {
  if (!wifiConnected || !firebaseReady) return;

  // 1. Kirim energy data ke /energy/block_a
  FirebaseJson energyJson;
  energyJson.set("voltage", blockA.voltage);
  energyJson.set("current", blockA.current);
  energyJson.set("power", blockA.power);
  energyJson.set("energy", blockA.energy);
  energyJson.set("timestamp", millis() / 1000);
  Firebase.RTDB.setJSON(&fbdo, "/energy/block_a", &energyJson);

  // 2. Kirim status lampu ke /lamp/status/a
  FirebaseJson statusJson;
  statusJson.set("state", lampAState);
  statusJson.set("timestamp", millis() / 1000);
  Firebase.RTDB.setJSON(&fbdo, "/lamp/status/a", &statusJson);
}

// ════════════════════════════════════════════════════════════════════════════
// Utility
// ════════════════════════════════════════════════════════════════════════════
void printResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset Reason: %d\n", reason);
}