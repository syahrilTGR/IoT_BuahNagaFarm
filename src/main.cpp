#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <WiFiManager.h>  // ESP32 WiFi Manager
#include <Firebase_ESP_Client.h>
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <RTClib.h>
RTC_DS3231 rtc;

// NTP time sync
#include "time.h"

// ── Pin Definition ───────────────────────────────────────────────────────────
static const uint8_t SSR_PIN = 14;          // GPIO14 → SSR (Blok A)
static const uint8_t SSR_PIN_B = 13;        // GPIO13 → SSR (Blok B)
static const uint8_t PZEM_RX = 33;          // ESP32 RX2 ← PZEM TX
static const uint8_t PZEM_TX = 32;          // ESP32 TX2 → PZEM RX
static const uint8_t PZEMB_RX = 26;         // Serial1 RX ← PZEM B RX
static const uint8_t PZEMB_TX = 25;         // Serial1 TX → PZEM B TX

// OLED I2C (default pins)
static const uint8_t OLED_SDA = 21;
static const uint8_t OLED_SCL = 22;
static const uint8_t OLED_ADDR = 0x3C;
static const uint16_t OLED_WIDTH = 128;
static const uint16_t OLED_HEIGHT = 64;

// Buzzer
static const uint8_t BUZZER_PIN = 27;        // GPIO27 → Buzzer (moved from 23, was noise-sensitive)

// ZMPT101B Voltage Sensor (nilai raw ADC, tanpa kalibrasi)
static const uint8_t ZMPT_PIN = 34;         // ADC1_CH6 (input only)
static const float ZMPT_THRESHOLD = 180.0;  // Threshold RMS: >180 = PLN, <=180 = UPS

// ── PZEM Objects ─────────────────────────────────────────────────────────────
// PZEM004Tv30 constructor akan panggil Serial2.begin() otomatis
PZEM004Tv30 pzemA(Serial2, PZEM_RX, PZEM_TX);  // Blok A, addr default 0xF8
PZEM004Tv30 pzemB(Serial1, PZEMB_RX, PZEMB_TX); // Blok B, addr default 0xF9

// OLED
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// ── Timers (non-blocking via millis) ─────────────────────────────────────────
struct Timer {
  unsigned long prev = 0;
  unsigned long interval = 0;
  Timer() = default;
  Timer(unsigned long p, unsigned long i) : prev(p), interval(i) {}
};

Timer timerPZEM      {0, 2500};   // Baca PZEM tiap 2 detik (skip saat PLN mati)
Timer timerFirebase  {0, 2500};   // Kirim ke Firebase tiap 2 detik
Timer timerOLED      {0, 1000};   // Update OLED tiap 1 detik
Timer timerCheckCmd  {0, 100};    // Cek perintah Firebase tiap 100ms
Timer timerBuzzer    {0, 0};      // Buzzer non-blocking
Timer timerZMPT      {0, 100};    // Baca ZMPT tiap 100ms

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
bool firebaseReady = false;
bool plnAlarmActive = false;
bool plnPrevState = true;   // assume PLN on at boot → first UPS detection triggers alarm
unsigned long lastPlnAlarm = 0;
static const int SYS_WIFI_INIT = 0;
  static const int SYS_WIFI_CONNECTING = 1;
  static const int SYS_WIFI_CHECK = 2;
  static const int SYS_WIFI_PORTAL = 3;
  static const int SYS_FIREBASE_INIT = 4;
  static const int SYS_FIREBASE_CHECK = 5;
  static const int SYS_READY = 6;
  int sysState = SYS_WIFI_INIT;
unsigned long stateStart = 0;

// Debounce: ignore Firebase commands within 2s after last execution

// Debounce: ignore Firebase commands within 2s after last execution
unsigned long lastCmdExecTimeA = 0;
unsigned long lastCmdExecTimeB = 0;
const unsigned long CMD_DEBOUNCE_MS = 2000;

// ── Buzzer state ─────────────────────────────────────────────────────────────
bool buzzerActive = false;
unsigned long buzzerDuration = 0;
unsigned long buzzerOffTime = 0;  // track when buzzer last turned OFF (for alarm silence gap)

// ZMPT voltage reading
float acVoltage = 0.0;

// ── Firebase objects ──────────────────────────────────────────────────────────────
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ── WiFiManager Instance ─────────────────────────────────────────────────────
WiFiManager wm;

// ── Forward Declarations ─────────────────────────────────────────────────────
void readPZEM();
void controlSSR();
void controlSSRB();
void setLampA(bool state);
void setLampB(bool state);
void updateOLED();
void checkFirebaseCommands();
void sendToFirebase();
unsigned long getEpochSec();
void printEnergyData(const EnergyData& d, const char* label);
void buzzerOn(unsigned long durationMs);
void readZMPT();
void syncRTC();

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
  // LED builtin sebagai indikator (active LOW)
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // I2C OLED — init awal, tampilkan boot screen
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] SSD1306 allocation failed");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("SmartNaga v2");
    display.println("Booting...");
    display.display();
    Serial.println("[OLED] Initialized");
  }

  // RTC init
  if (!rtc.begin()) {
    Serial.println("[RTC] Not found");
    // set to compile time if RTC missing
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } else {
    Serial.println("[RTC] Initialized");
  }

  // Serial2 sudah di-init otomatis oleh PZEM004Tv30 constructor

// Default lampu OFF saat boot (sebelum sync Firebase)
  lampAState = false;
  lampBState = false;
  controlSSR();
  controlSSRB();

  // WiFi & Firebase: non-blocking (dilakukan di loop())
  wifiConnected = false;
  firebaseReady = false;

  Serial.println("=== Setup Selesai (WiFi+Firebase non-blocking) ===\n");
  buzzerOn(200);
}

void loop() {
  unsigned long now = millis();

  // Non-blocking WiFi + Firebase init (state machine)
  static bool firebaseInitialized = false;
  if (!firebaseInitialized) {
    switch (sysState) {
      case SYS_WIFI_INIT:
        wm.setConfigPortalTimeout(180);
        wm.setConnectTimeout(30);
        sysState = SYS_WIFI_CONNECTING;
        stateStart = now;
        break;

      case SYS_WIFI_CONNECTING:
        Serial.println("[WiFi] Connecting (non-blocking)...");
        WiFi.begin();
        sysState = SYS_WIFI_CHECK;
        stateStart = now;
        break;

      case SYS_WIFI_CHECK:
        if (WiFi.status() == WL_CONNECTED) {
          wifiConnected = true;
          Serial.print("[WiFi] Connected! IP: ");
          Serial.println(WiFi.localIP());
          sysState = SYS_FIREBASE_INIT;
        } else if (now - stateStart >= 30000) {
          // Fallback ke Config Portal AP
          Serial.println("[WiFi] Connect failed, starting Config Portal...");
          sysState = SYS_WIFI_PORTAL;
          stateStart = now;
        }
        break;

      case SYS_WIFI_PORTAL:
        Serial.println("[WiFi] Starting Config Portal (AP mode)...");
        {
          String apName = String("SmartNaga-") + String((uint32_t)ESP.getEfuseMac(), HEX);
          bool res = wm.startConfigPortal(apName.c_str());
          if (res) {
            wifiConnected = true;
            Serial.print("[WiFi] Connected via Portal! IP: ");
            Serial.println(WiFi.localIP());
            sysState = SYS_FIREBASE_INIT;
          } else {
            Serial.println("[WiFi] Config Portal failed, restart...");
            delay(3000);
            ESP.restart();
          }
        }
        break;

      case SYS_FIREBASE_INIT:
        Serial.println("[Firebase] Initializing...");
        config.api_key = FIREBASE_API_KEY;
        config.database_url = FIREBASE_DATABASE_URL;
        auth.user.email = FIREBASE_USER_EMAIL;
        auth.user.password = FIREBASE_USER_PASSWORD;
        Firebase.begin(&config, &auth);
        Firebase.reconnectWiFi(true);
        configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        sysState = SYS_FIREBASE_CHECK;
        stateStart = now;
        break;

      case SYS_FIREBASE_CHECK:
        if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
          firebaseReady = true;
          Serial.println("[Firebase] Connected & Authenticated");

          // Init default control paths
          Firebase.RTDB.setBool(&fbdo, "/lamp/control/a", false);
          Firebase.RTDB.setBool(&fbdo, "/lamp/control/b", false);

          // Sync initial lamp state
          bool stateA = false, stateB = false;
          Firebase.RTDB.getBool(&fbdo, "/lamp/control/a");
          if (fbdo.boolData()) stateA = fbdo.boolData();
          Firebase.RTDB.getBool(&fbdo, "/lamp/control/b");
          if (fbdo.boolData()) stateB = fbdo.boolData();
          setLampA(stateA);
          setLampB(stateB);

          sysState = SYS_READY;
          firebaseInitialized = true;
          Serial.println("[System] Ready!");

          syncRTC();  // Sync RTC time from NTP
        } else if (now - stateStart >= 20000) {
          Serial.println("[Firebase] Timeout, retry later...");
          sysState = SYS_READY;
          firebaseInitialized = true;
        }
        break;

      default:
        sysState = SYS_READY;
        firebaseInitialized = true;
        break;
    }
  }

  // 0. Buzzer + PLN alarm — PALING PERTAMA, sebelum semua blocking call
  //   Pakai millis() langsung (bukan `now`) karena `now` bisa stale
  if (buzzerActive && (millis() - timerBuzzer.prev >= buzzerDuration)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    buzzerOffTime = millis();
  }

  // 0b. PLN alarm repeat — require 1s minimum silence after buzzer OFF
  if (plnAlarmActive && !buzzerActive && (millis() - lastPlnAlarm >= 3000) && (millis() - buzzerOffTime >= 1000)) {
    lastPlnAlarm = millis();
    buzzerOn(500);
  }

  // 1. Baca PZEM tiap 5 detik — skip saat PLN mati (PZEM AC-powered, read akan block ~7s)
  if (now - timerPZEM.prev >= timerPZEM.interval && acVoltage > ZMPT_THRESHOLD) {
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

  // 4. Kirim data ke Firebase tiap 1 detik
  if (now - timerFirebase.prev >= timerFirebase.interval) {
    timerFirebase.prev = now;
    sendToFirebase();
  }

  // 5. Update OLED tiap 1 detik
  if (now - timerOLED.prev >= timerOLED.interval) {
    timerOLED.prev = now;
    updateOLED();
  }

  // 6. Baca ZMPT tiap 100ms
  if (now - timerZMPT.prev >= timerZMPT.interval) {
    timerZMPT.prev = now;
    readZMPT();
  }

  // Watchdog / keep WiFi alive
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
  } else if (!wifiConnected) {
    wifiConnected = true;
    Serial.println("WiFi Reconnected");
  }
}


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
  buzzerOn(200);
  Serial.printf("[SSR] Lampu Blok A: %s\n", state ? "ON" : "OFF");
}

void setLampB(bool state) {
  lampBState = state;
  controlSSRB();
  buzzerOn(200);
  Serial.printf("[SSR] Lampu Blok B: %s\n", state ? "ON" : "OFF");
}

// ════════════════════════════════════════════════════════════════════════════
// OLED Display (placeholder - implement nanti)
// ════════════════════════════════════════════════════════════════════════════
void updateOLED() {
  display.clearDisplay();

  // Line 1: WiFi + Power source
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(wifiConnected ? "WiFi: ON" : "WiFi: --");

  const char* source = "UPS";
  if (acVoltage > ZMPT_THRESHOLD) source = "PLN";

  display.setCursor(70, 0);
  display.printf("Src:%s", source);

  // Line 2: Waktu (RTC)
  display.setCursor(0, 16);
  DateTime now = rtc.now();
  display.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  // Line 3-4: Power A dan B vertikal (atas-bawah), text size 2
  display.setTextSize(2);

  float powerA = blockA.valid ? blockA.power : 0.0;
  float powerB = blockB.valid ? blockB.power : 0.0;

  display.setCursor(0, 28);
  display.printf("A:%.0fW", powerA);

  display.setCursor(0, 46);
  display.printf("B:%.0fW", powerB);

  display.display();
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
    if (cmd != lampAState && (now - lastCmdExecTimeA >= CMD_DEBOUNCE_MS)) {
      setLampA(cmd);
      lastCmdExecTimeA = now;
      Serial.println("[Firebase] Command executed Blok A.");
    }
  }

  // Baca /lamp/control/b (boolean)
  if (Firebase.RTDB.getBool(&fbdo, "/lamp/control/b")) {
    bool cmd = fbdo.boolData();
    if (cmd != lampBState && (now - lastCmdExecTimeB >= CMD_DEBOUNCE_MS)) {
      setLampB(cmd);
      lastCmdExecTimeB = now;
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

  // Kirim status PLN + raw RMS ke /energy/z_mpt
  FirebaseJson zMptJson;
  zMptJson.set("pln", acVoltage > ZMPT_THRESHOLD);
  zMptJson.set("voltage_raw", acVoltage);
  zMptJson.set("timestamp", (double)ts);
  Firebase.RTDB.setJSON(&fbdo, "/energy/z_mpt", &zMptJson);

  // Kirim status PLN terpisah untuk notifikasi Cloud Function
  FirebaseJson plnJson;
  bool plnAvailable = acVoltage > ZMPT_THRESHOLD;
  plnJson.set("available", plnAvailable);
  plnJson.set("source", plnAvailable ? "PLN" : "UPS");
  plnJson.set("timestamp", (double)ts);
  Firebase.RTDB.setJSON(&fbdo, "/energy/pln_status", &plnJson);

  // Heartbeat: device online status (updated tiap 1s bersama energy data)
  Firebase.RTDB.setInt(&fbdo, "/device/heartbeat", (int)ts);

  // Note: /lamp/status/{a,b} ditulis oleh aplikasi (updateLampControl)
  // Firmware hanya baca /lamp/control/{a,b} untuk eksekusi perintah
}

// ════════════════════════════════════════════════════════════════════════════
// Utility
// ════════════════════════════════════════════════════════════════════════════
// ════════════════════════════════════════════════════════════════════════════
// Utility
// ════════════════════════════════════════════════════════════════════════════
unsigned long getEpochSec() {
  time_t t = time(nullptr);
  if (t > 1000000000) return (unsigned long)t;
  return millis() / 1000;
}

void printResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset Reason: %d\n", reason);
}

// Sync RTC dari NTP (panggil sekali setelah WiFi connected)
void syncRTC() {
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  delay(2000);  // tunggu NTP sync
  time_t t = time(nullptr);
  if (t > 1000000000) {
    struct tm* tm = localtime(&t);
    rtc.adjust(DateTime(
      tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
      tm->tm_hour, tm->tm_min, tm->tm_sec
    ));
    Serial.printf("[RTC] Synced from NTP: %04d-%02d-%02d %02d:%02d:%02d\n",
      tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
      tm->tm_hour, tm->tm_min, tm->tm_sec);
  } else {
    Serial.println("[RTC] NTP sync failed");
  }
}

// ════════════════════════════════════════════════════════════════════════════
// Buzzer (non-blocking)
// ════════════════════════════════════════════════════════════════════════════
void buzzerOn(unsigned long durationMs) {
  digitalWrite(BUZZER_PIN, HIGH);
  buzzerActive = true;
  buzzerDuration = durationMs;
  timerBuzzer.prev = millis();
  Serial.printf("[%lu][BUZZ] %lums\n", millis()/1000, durationMs);
}

// ════════════════════════════════════════════════════════════════════════════
// ZMPT101B Voltage Sensor
// ════════════════════════════════════════════════════════════════════════════
void readZMPT() {
  // Safety net: cek buzzer-OFF sebelum blocking ADC sampling
  unsigned long ms = millis();
  if (buzzerActive && (ms - timerBuzzer.prev >= buzzerDuration)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    buzzerOffTime = ms;
  }

  // Baca ADC untuk baseline (rata-rata DC offset)
  long sumRaw = 0;
  for (int i = 0; i < 200; i++) {
    sumRaw += analogRead(ZMPT_PIN);
    delayMicroseconds(50);
  }
  float baseline = sumRaw / 200.0;

  // Baca ADC untuk RMS (200 samples)
  long sum = 0;
  int minVal = 4095, maxVal = 0;
  for (int i = 0; i < 200; i++) {
    int raw = analogRead(ZMPT_PIN);
    int offset = raw - (int)baseline;
    sum += (long)offset * offset;
    if (raw < minVal) minVal = raw;
    if (raw > maxVal) maxVal = raw;
    delayMicroseconds(100);
  }
  float rms = sqrt(sum / 200.0);
  int peakToPeak = maxVal - minVal;

  // Deteksi invalid:
  // 1. RMS < 30 = noise floor (tidak ada tegangan AC)
  // 2. peak-to-peak < 20 = pin floating atau DC saja
  // 3. RMS > 500 = ADC saturate
  bool plnAvailable = false;
  if (rms >= 30.0 && peakToPeak >= 20 && rms <= 500.0 && rms > ZMPT_THRESHOLD) {
    plnAvailable = true;
  }

  // acVoltage diisi RMS raw (tanpa kalibrasi) untuk display
  acVoltage = (plnAvailable || rms >= 30.0) ? rms : 0.0;

  // Print ke Serial Monitor tiap ~5 detik (throttle)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 5000) {
    lastPrint = millis();
    const char* src = plnAvailable ? "PLN" : "UPS";
    Serial.printf("[%lu][ZMPT] RMS:%.1f P2P:%d Base:%.1f Src:%s\n", millis()/1000, rms, peakToPeak, baseline, src);
  }

  // Alarm PLN mati: transisi PLN -> UPS = beep 500ms
  if (plnPrevState && !plnAvailable) {
    plnAlarmActive = true;
    lastPlnAlarm = millis();
    buzzerOn(500);
    Serial.printf("[%lu][ALARM] PLN MATI!\n", millis()/1000);
  } else if (!plnPrevState && plnAvailable) {
    plnAlarmActive = false;
    Serial.printf("[%lu][ALARM] PLN KEMBALI\n", millis()/1000);
  }
  plnPrevState = plnAvailable;
}