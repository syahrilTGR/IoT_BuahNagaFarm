#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <WiFiManager.h>  // ESP32 WiFi Manager
// #include "config.h"     // Uncomment saat Firebase di-integrasikan ke production

// ── Pin Definition ───────────────────────────────────────────────────────────
static const uint8_t SSR_PIN = 13;          // D13 → LLC → SSR (Blok A)
static const uint8_t SSR_PIN_B = 12;        // D12 → LLC → SSR (Blok B) - future
static const uint8_t PZEM_RX = 16;          // RX2 (GPIO16) ← PZEM TX
static const uint8_t PZEM_TX = 17;          // TX2 (GPIO17) → PZEM RX

// ── PZEM Objects ─────────────────────────────────────────────────────────────
PZEM004Tv30 pzemA(Serial2, PZEM_RX, PZEM_TX);  // Blok A
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

// ── WiFiManager Instance ─────────────────────────────────────────────────────
WiFiManager wm;

// ── Forward Declarations ─────────────────────────────────────────────────────
void setupWiFi();
void readPZEM();
void controlSSR();
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

  // Serial2 for PZEM (RX2=16, TX2=17)
  Serial2.begin(9600, SERIAL_8N1, PZEM_RX, PZEM_TX);
  delay(100);

  // WiFi Manager (auto-connect atau AP config portal)
  setupWiFi();

  Serial.println("=== Setup Selesai ===\n");
}

void loop() {
  unsigned long now = millis();

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
// PZEM Reading
// ════════════════════════════════════════════════════════════════════════════
void readPZEM() {
  blockA.voltage = pzemA.voltage();
  blockA.current = pzemA.current();
  blockA.power   = pzemA.power();
  blockA.energy  = pzemA.energy();

  // Validasi pembacaan
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
// Firebase Integration (placeholder - implement nanti)
// ════════════════════════════════════════════════════════════════════════════
void checkFirebaseCommands() {
  // TODO: Implement Firebase RTDB listener
  // Path yang didengarkan:
  // - /lamp/control/a  (boolean)
  // - /lamp/control/b  (boolean)

  // Contoh logic nanti:
  // if (firebaseData.changed("/lamp/control/a")) {
  //   bool cmd = firebaseData.boolData("/lamp/control/a");
  //   setLampA(cmd);
  // }
}

void sendToFirebase() {
  if (!wifiConnected) return;

  // TODO: Implement Firebase RTDB push
  // Path:
  // - /energy/block_a  {voltage, current, power, energy, timestamp}
  // - /lamp/status/a   {state: true/false, timestamp}

  // Serial debug untuk sekarang
  Serial.println("[Firebase] Data siap kirim (placeholder)");
}

// ════════════════════════════════════════════════════════════════════════════
// Utility
// ════════════════════════════════════════════════════════════════════════════
void printResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset Reason: %d\n", reason);
}