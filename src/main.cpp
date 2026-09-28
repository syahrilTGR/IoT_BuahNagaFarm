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

// Ping (ESP-IDF esp_ping) — untuk cek konektivitas internet
#include "ping/ping_sock.h"

// ── Pin Definition ───────────────────────────────────────────────────────────
static const uint8_t SSR_PIN = 14;          // GPIO14 → SSR (Blok A)
static const uint8_t SSR_PIN_B = 13;        // GPIO13 → SSR (Blok B)
static const uint8_t PZEM_RX = 33;          // ESP32 RX2 ← PZEM A TX
static const uint8_t PZEM_TX = 32;          // ESP32 TX2 → PZEM A RX
static const uint8_t PZEMB_RX = 26;         // Serial1 RX ← PZEM B TX
static const uint8_t PZEMB_TX = 25;         // Serial1 TX → PZEM B RX

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
static const float ZMPT_THRESHOLD = 150.0;  // Threshold RMS: <150 = UPS, >=150 = PLN

// ── ZMPT Kalibrasi & Sampling ────────────────────────────────────────────────
// Sampling SATU jendela (baseline + RMS dihitung dari buffer yang SAMA).
// analogRead() ~9.5us + delay 70us = ~80us/sampel
// 600 sampel × 80us = ~48ms = ~2.4 siklus jala-jala 50Hz.
static const int ZMPT_SAMPLES = 600;
static const int ZMPT_SAMPLE_DELAY_US = 70;

// Debounce state PLN: butuh N pembacaan konsisten sebelum ganti state.
static const int PLN_DEBOUNCE_READS = 3;

// Faktor konversi RMS-ADC → Volt (hasil kalibrasi vs PZEM).
// Data 10 pasang (2026-09-25): mean RMS 356.73, mean PZEM 238.07
// → K = 238.07 / 356.73 = 0.667
static const float ZMPT_K = 0.667;

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
Timer timerResetWifi {0, 3000};   // Cek perintah reset WiFi tiap 3 detik
Timer timerPing      {0, 3000};   // Ping 8.8.8.8 tiap 3 detik
Timer timerConfig    {0, 10000};  // Baca /config/block_{a,b} tiap 10 detik

// ── State ────────────────────────────────────────────────────────────────────
struct EnergyData {
  float voltage = 0;
  float current = 0;
  float power = 0;
  float energy = 0;
  bool valid = false;
  unsigned long lastGoodMs = 0;  // millis() saat pembacaan valid terakhir (freshness gate)
};

// ── Config dari Firebase (/config/block_{a,b}) ───────────────────────────────
// Dipakai decision tree. Ditulis app (AnomalyConfigScreen), firmware hanya baca.
// Default mengikuti app: expected 0 (= belum dikonfigurasi), lampCount 0, tol 15%.
struct LampConfig {
  float expectedPower = 0.0f;
  int lampCount = 0;
  float tolerancePercent = 15.0f;
  bool loaded = false;          // pernah berhasil baca dari Firebase
  unsigned long lastReadMs = 0;
};

// ── Status diagnosa lampu (port dari LampDiagnosticStatus app) ───────────────
// Urutan enum tidak penting; yang penting PRECEDENCE di evaluateLampStatusFw().
enum class LampStatusFw : uint8_t {
  FW_SENSOR_ERROR,
  FW_PLN_OUTAGE,
  FW_UNDER_VOLTAGE,
  FW_OVER_VOLTAGE,
  FW_NORMAL_STANDBY,
  FW_NORMAL_ACTIVE,
  FW_CIRCUIT_BROKEN,
  FW_PARTIAL_LAMP_FAIL,
  FW_OVERLOAD,
  FW_RELAY_LEAKAGE
};

struct LampDiag {
  LampStatusFw status = LampStatusFw::FW_NORMAL_STANDBY;
  int brokenLamps = 0;
  bool isAlert = false;
  bool settling = false;
};

EnergyData blockA;
EnergyData blockB;
LampConfig configA;
LampConfig configB;
LampDiag diagA;
LampDiag diagB;
unsigned long lampToggleMsA = 0;  // waktu toggle terakhir (untuk settling 4s)
unsigned long lampToggleMsB = 0;
bool lampAState = false;
bool lampBState = false;
bool wifiConnected = false;
bool firebaseReady = false;
bool plnPrevState = true;     // state PLN saat ini (di-debounce)
bool plnStateInit = false;    // false = belum ada pembacaan valid (cegah alarm palsu saat boot)
bool inConfigPortal = false;  // flag untuk indikator Config Portal di OLED
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

// ── Ping state ───────────────────────────────────────────────────────────────
bool lastPingOk = false;
unsigned long lastPingMs = 0;  // latency dalam ms
IPAddress pingTarget(8, 8, 8, 8);  // Google Public DNS

// ── ZMPT state ───────────────────────────────────────────────────────────────
// Buffer 600 sampel: baseline + RMS dihitung dari jendela yang SAMA
static int zmptBuffer[ZMPT_SAMPLES];

// Debounce state PLN (butuh N pembacaan konsisten sebelum ganti state)
static int zmptPlnReads = 0;
static int zmptUpsReads = 0;

// ZMPT voltage reading (RMS mentah + volt terkalibrasi)
float acVoltage = 0.0;      // RMS ADC mentah (untuk threshold & Firebase voltage_raw)
float acVoltageReal = 0.0;  // volt terkalibrasi (RMS * ZMPT_K)

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
void checkWifiResetCommand();
void pingGoogle();
void readConfigOnce(const char* path, LampConfig& cfg);
void updateDiagnostics();
LampStatusFw evaluateLampStatusFw(bool relayOn, float voltage, float current, float power, const LampConfig& cfg, bool sensorOk);
const char* lampStatusName(LampStatusFw s);
const char* lampStatusShortLabel(LampStatusFw s);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== SmartNaga ESP32 Boot ===");

  // SSR pin — active LOW: LOW = ON, HIGH = OFF
  // Set HIGH dulu (selagi INPUT) agar saat pinMode(OUTPUT) pin TIDAK glitch LOW
  digitalWrite(SSR_PIN, HIGH);   // HIGH = OFF (active LOW)
  pinMode(SSR_PIN, OUTPUT);

  digitalWrite(SSR_PIN_B, HIGH);
  pinMode(SSR_PIN_B, OUTPUT);

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
        Serial.println("[WiFi] SSID: SmartNaga-AP  |  Pass: smartnaga123  |  IP: 192.168.4.1");
        inConfigPortal = true;
        {
          // Tampilkan indikator di OLED sebelum portal (blocking)
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0, 0);
          display.println("== SETUP WIFI ==");
          display.println("");
          display.println("1. Konek ke WiFi:");
          display.println("   SmartNaga-AP");
          display.println("2. Pass: smartnaga123");
          display.println("3. Buka 192.168.4.1");
          display.display();

          bool res = wm.startConfigPortal("SmartNaga-AP", "smartnaga123");
          if (res) {
            wifiConnected = true;
            inConfigPortal = false;
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

  // 0. Buzzer OFF check — PALING PERTAMA, sebelum semua blocking call
  //   Pakai millis() langsung (bukan `now`) karena `now` bisa stale
  if (buzzerActive && (millis() - timerBuzzer.prev >= buzzerDuration)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    buzzerOffTime = millis();
  }

  // 1. Baca PZEM tiap 5 detik — skip saat PLN mati (PZEM AC-powered, read akan block ~7s)
  if (now - timerPZEM.prev >= timerPZEM.interval && plnPrevState) {
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

  // 3b. Cek perintah reset WiFi tiap 3 detik
  if (now - timerResetWifi.prev >= timerResetWifi.interval) {
    timerResetWifi.prev = now;
    checkWifiResetCommand();
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

  // 7. Ping google.com tiap 3 detik (non-blocking)
  if (now - timerPing.prev >= timerPing.interval) {
    timerPing.prev = now;
    pingGoogle();
  }

  // 8. Baca config decision tree dari Firebase
  //    One-shot saat Firebase pertama siap (jangan tunggu 5 menit), lalu periodik.
  //    timerConfig.prev di-set SEBELUM read supaya read yang lambat tidak
  //    memicu ulang di iterasi berikutnya.
  if (firebaseReady) {
    static bool configBootRead = false;
    if (!configBootRead) {
      configBootRead = true;
      timerConfig.prev = now;
      readConfigOnce("/config/block_a", configA);
      readConfigOnce("/config/block_b", configB);
    } else if (now - timerConfig.prev >= timerConfig.interval) {
      timerConfig.prev = now;
      readConfigOnce("/config/block_a", configA);
      readConfigOnce("/config/block_b", configB);
    }
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
  // Freshness: catat waktu pembacaan valid — HANYA di sini, tidak di tempat lain.
  if (blockB.valid) blockB.lastGoodMs = millis();

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
  if (blockA.valid) blockA.lastGoodMs = millis();

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
  lampToggleMsA = millis();  // catat waktu toggle untuk settling 4 detik
  Serial.printf("[SSR] Lampu Blok A: %s\n", state ? "ON" : "OFF");
}

void setLampB(bool state) {
  lampBState = state;
  controlSSRB();
  buzzerOn(200);
  lampToggleMsB = millis();  // catat waktu toggle untuk settling 4 detik
  Serial.printf("[SSR] Lampu Blok B: %s\n", state ? "ON" : "OFF");
}

// ════════════════════════════════════════════════════════════════════════════
// Decision Tree lampu — PORT PRESISI dari app (LampDiagnosticStatus)
//
// Sumber kebenaran logika: smartnaga_app/lib/core/algorithms/decision_tree_evaluator.dart
// Threshold di bawah HARUS identik dengan app. Kalau salah satu diubah, ubah
// di KEDUA repo dalam satu commit — kalau tidak, OLED dan app bisa beda label.
//
// Kenapa di firmware: app tidak jalan saat ditutup, sedangkan OLED harus
// selalu benar. Firmware = satu-satunya aktor yang selalu hidup.
// ════════════════════════════════════════════════════════════════════════════

// ── Threshold (identik dengan decision_tree_evaluator.dart) ─────────────────
static const float LEAK_POWER_W        = 2.0f;    // relayLeakage: daya
static const float LEAK_CURRENT_A      = 0.05f;   // relayLeakage: arus
static const float PLN_OUTAGE_V        = 50.0f;   // plnOutage
static const float UNDER_VOLTAGE_V     = 190.0f;  // underVoltage
static const float OVER_VOLTAGE_V      = 245.0f;  // overVoltage
static const float NO_CONFIG_MIN_W     = 2.0f;    // circuitBroken saat expected<=0
static const float DEFAULT_PPL_W       = 9.0f;    // daya per lamp fallback
static const float BROKEN_FRACTION     = 0.3f;    // circuitBroken saat expected>0
static const float TOLERANCE_DEFAULT   = 15.0f;   // % toleransi

// Freshness: pembacaan dianggap basi setelah 4 siklus PZEM terlewat (2.5s × 4).
// readPZEM() di-SKIP saat PLN mati, jadi blockX.valid tetap true dari bacaan
// terakhir — tanpa gate ini, daya basi bisa tampil sebagai fresh.
static const unsigned long ENERGY_STALE_MS = 10000;

// Settling setelah toggle relay: PZEM baru update tiap 2.5s, jadi nilai tepat
// setelah toggle belum mencerminkan kondisi baru. Tahan evaluasi 4 detik.
static const unsigned long TOGGLE_SETTLING_MS = 4000;

// Grace period setelah boot: sensor belum tentu siap, jangan langsung
// laporkan sensorError.
static const unsigned long SENSOR_GRACE_MS = 10000;

// ── Config dari Firebase ────────────────────────────────────────────────────
void readConfigOnce(const char* path, LampConfig& cfg) {
  if (!wifiConnected || !firebaseReady) return;

  FirebaseJson fbjson;
  if (Firebase.RTDB.getJSON(&fbdo, path, &fbjson)) {
    FirebaseJsonData d;
    float ep = cfg.expectedPower;   // pertahankan nilai lama kalau field absen
    int   lc = cfg.lampCount;
    float tp = cfg.tolerancePercent;

    if (fbjson.get(d, "expected_power"))    ep = d.floatValue;
    if (fbjson.get(d, "lamp_count"))        lc = d.intValue;
    if (fbjson.get(d, "tolerance_percent")) tp = d.floatValue;

    // Kalau node ada tapi kosong, pakai default app
    if (tp <= 0.0f) tp = TOLERANCE_DEFAULT;

    cfg.expectedPower    = ep;
    cfg.lampCount        = lc;
    cfg.tolerancePercent = tp;
    cfg.loaded           = true;
    cfg.lastReadMs       = millis();

    Serial.printf("[CFG] %s -> expected=%.0fW lamps=%d tol=%.0f%%\n",
                  path, ep, lc, tp);
  } else {
    // Gagal baca (network/node absen): PERTAHANKAN config lama, jangan nol-kan.
    // Config yang sudah diketahui baik tidak boleh hilang karena blip jaringan.
    Serial.printf("[CFG] %s read failed: %s (keep last)\n",
                  path, fbdo.errorReason().c_str());
  }
}

// ── Evaluator (port presisi decision_tree_evaluator.dart:127-262) ───────────
LampStatusFw evaluateLampStatusFw(bool relayOn, float voltage, float current,
                                  float power, const LampConfig& cfg, bool sensorOk) {
  // ROOT: sensor tidak tersedia (prioritas tertinggi)
  if (!sensorOk) return LampStatusFw::FW_SENSOR_ERROR;

  // NODE 1: relay OFF — lampu seharusnya mati
  if (!relayOn) {
    if (power > LEAK_POWER_W || current > LEAK_CURRENT_A) {
      return LampStatusFw::FW_RELAY_LEAKAGE;
    }
    return LampStatusFw::FW_NORMAL_STANDBY;
  }

  // NODE 2: relay ON — cek kualitas tegangan (pakai tegangan PZEM)
  if (voltage < PLN_OUTAGE_V)   return LampStatusFw::FW_PLN_OUTAGE;
  if (voltage < UNDER_VOLTAGE_V) return LampStatusFw::FW_UNDER_VOLTAGE;
  if (voltage > OVER_VOLTAGE_V)  return LampStatusFw::FW_OVER_VOLTAGE;

  // NODE 3: tegangan normal — evaluasi konsumsi daya
  if (cfg.expectedPower <= 0.0f) {
    // Config belum diisi: status dasar dari daya saja
    if (power < NO_CONFIG_MIN_W) return LampStatusFw::FW_CIRCUIT_BROKEN;
    return LampStatusFw::FW_NORMAL_ACTIVE;
  }

  const float powerPerLamp = (cfg.lampCount > 0)
      ? (cfg.expectedPower / (float)cfg.lampCount)
      : DEFAULT_PPL_W;
  const float lower = cfg.expectedPower * (1.0f - cfg.tolerancePercent / 100.0f);
  const float upper = cfg.expectedPower * (1.0f + cfg.tolerancePercent / 100.0f);

  // Daya nyaris nol padahal relay ON & tegangan normal -> jalur putus
  if (power < powerPerLamp * BROKEN_FRACTION) return LampStatusFw::FW_CIRCUIT_BROKEN;

  // Dalam rentang toleransi -> normal
  if (power >= lower && power <= upper) return LampStatusFw::FW_NORMAL_ACTIVE;

  // Di bawah batas bawah -> sebagian lampu mati
  if (power < lower) return LampStatusFw::FW_PARTIAL_LAMP_FAIL;

  // Catch-all: di atas batas atas -> beban berlebih
  return LampStatusFw::FW_OVERLOAD;
}

// ── Jumlah lampu mati (estimasi, clamp 1..lampCount) ────────────────────────
int estimateDeadLamps(float power, const LampConfig& cfg) {
  if (cfg.expectedPower <= 0.0f) return 0;
  const float powerPerLamp = (cfg.lampCount > 0)
      ? (cfg.expectedPower / (float)cfg.lampCount)
      : DEFAULT_PPL_W;
  if (powerPerLamp <= 0.0f) return 0;
  int dead = (int)lroundf((cfg.expectedPower - power) / powerPerLamp);
  if (dead < 1) dead = 1;
  if (cfg.lampCount > 0 && dead > cfg.lampCount) dead = cfg.lampCount;
  return dead;
}

// ── Nama status (string enum Dart — untuk wire/RTDB) ────────────────────────
const char* lampStatusName(LampStatusFw s) {
  switch (s) {
    case LampStatusFw::FW_SENSOR_ERROR:      return "sensorError";
    case LampStatusFw::FW_PLN_OUTAGE:        return "plnOutage";
    case LampStatusFw::FW_UNDER_VOLTAGE:     return "underVoltage";
    case LampStatusFw::FW_OVER_VOLTAGE:      return "overVoltage";
    case LampStatusFw::FW_NORMAL_STANDBY:    return "normalStandby";
    case LampStatusFw::FW_NORMAL_ACTIVE:     return "normalActive";
    case LampStatusFw::FW_CIRCUIT_BROKEN:    return "circuitBroken";
    case LampStatusFw::FW_PARTIAL_LAMP_FAIL: return "partialLampFail";
    case LampStatusFw::FW_OVERLOAD:          return "overload";
    case LampStatusFw::FW_RELAY_LEAKAGE:     return "relayLeakage";
  }
  return "unknown";
}

// ── Label OLED (pendek; maks 13 char agar muat "Status: XXXXX" = 20 char) ───
const char* lampStatusShortLabel(LampStatusFw s) {
  switch (s) {
    case LampStatusFw::FW_SENSOR_ERROR:      return "Sensor Error";
    case LampStatusFw::FW_PLN_OUTAGE:        return "Listrik Padam";
    case LampStatusFw::FW_UNDER_VOLTAGE:     return "Teg. Rendah";
    case LampStatusFw::FW_OVER_VOLTAGE:      return "Teg. Tinggi";
    case LampStatusFw::FW_NORMAL_STANDBY:    return "Standby";
    case LampStatusFw::FW_NORMAL_ACTIVE:     return "Normal";
    case LampStatusFw::FW_CIRCUIT_BROKEN:    return "Lampu Padam";
    case LampStatusFw::FW_PARTIAL_LAMP_FAIL: return "Lampu Rusak";
    case LampStatusFw::FW_OVERLOAD:          return "Beban Lebih";
    case LampStatusFw::FW_RELAY_LEAKAGE:     return "Relay Bocor";
  }
  return "?";
}

// ── Baris status OLED: label + jumlah lampu rusak (kalau ada) ───────────────
// Lebar baris size 1 = 21 char (126px) dari 128px.
// "Status: Lampu Rusak" = 20 char (120px) — sudah mepet; menambah " (2)"
// jadi 23 char (138px) OVER. Karena itu untuk partialLampFail label
// dipendekkan jadi "Rusak" → "Status: Rusak (2)" = 17 char (102px), muat.
// Sumber angka: diagX.brokenLamps (dihitung estimateDeadLamps(), clamp 1..lampCount).
void formatStatusLine(const LampDiag& d, char* buf, size_t n) {
  if (d.status == LampStatusFw::FW_PARTIAL_LAMP_FAIL && d.brokenLamps > 0) {
    snprintf(buf, n, "Status: Rusak (%d)", d.brokenLamps);
  } else {
    snprintf(buf, n, "Status: %s", lampStatusShortLabel(d.status));
  }
}

// ── Hitung diagA/diagB dari state saat ini ─────────────────────────────────
// Dipanggil dari updateOLED() tiap 1 detik, jadi tetap fresh walau WiFi/Firebase
// mati (sendToFirebase() early-return).
void updateDiagnostics() {
  const unsigned long now = millis();

  // Grace period: sejak ZMPT pertama valid
  static unsigned long firstZMPTMs = 0;
  if (plnStateInit && firstZMPTMs == 0) firstZMPTMs = now;
  const bool inGrace = (firstZMPTMs == 0) || (now - firstZMPTMs < SENSOR_GRACE_MS);

  // Freshness: pembacaan PZEM masih baru?
  const bool freshA = (blockA.lastGoodMs != 0) && (now - blockA.lastGoodMs < ENERGY_STALE_MS);
  const bool freshB = (blockB.lastGoodMs != 0) && (now - blockB.lastGoodMs < ENERGY_STALE_MS);

  // Nilai efektif: nol kalau tidak bisa dipercaya (PLN mati / basi / invalid)
  const bool usableA = plnPrevState && blockA.valid && freshA;
  const bool usableB = plnPrevState && blockB.valid && freshB;
  const float vA = usableA ? blockA.voltage : 0.0f;
  const float cA = usableA ? blockA.current : 0.0f;
  const float pA = usableA ? blockA.power   : 0.0f;
  const float vB = usableB ? blockB.voltage : 0.0f;
  const float cB = usableB ? blockB.current : 0.0f;
  const float pB = usableB ? blockB.power   : 0.0f;

  // sensorOk: PLN mati BUKAN sensor error (sensor lokal sehat, sumbernya hilang).
  // Tanpa suku !plnPrevState, PLN padam akan salah dilaporkan sebagai sensorError.
  const bool sensorOkA = inGrace || !plnPrevState || (blockA.valid && freshA);
  const bool sensorOkB = inGrace || !plnPrevState || (blockB.valid && freshB);

  // Settling: tahan status lama selama 4 detik setelah toggle
  const bool settlingA = (now - lampToggleMsA) < TOGGLE_SETTLING_MS;
  const bool settlingB = (now - lampToggleMsB) < TOGGLE_SETTLING_MS;

  LampDiag na, nb;

  if (settlingA) {
    na = diagA;                 // pertahankan hasil sebelum toggle
    na.settling = true;
  } else {
    na.status = evaluateLampStatusFw(lampAState, vA, cA, pA, configA, sensorOkA);
    na.brokenLamps = (na.status == LampStatusFw::FW_PARTIAL_LAMP_FAIL)
                     ? estimateDeadLamps(pA, configA)
                     : ((na.status == LampStatusFw::FW_CIRCUIT_BROKEN) ? configA.lampCount : 0);
    na.isAlert = !(na.status == LampStatusFw::FW_NORMAL_ACTIVE ||
                   na.status == LampStatusFw::FW_NORMAL_STANDBY);
    na.settling = false;
  }

  if (settlingB) {
    nb = diagB;
    nb.settling = true;
  } else {
    nb.status = evaluateLampStatusFw(lampBState, vB, cB, pB, configB, sensorOkB);
    nb.brokenLamps = (nb.status == LampStatusFw::FW_PARTIAL_LAMP_FAIL)
                     ? estimateDeadLamps(pB, configB)
                     : ((nb.status == LampStatusFw::FW_CIRCUIT_BROKEN) ? configB.lampCount : 0);
    nb.isAlert = !(nb.status == LampStatusFw::FW_NORMAL_ACTIVE ||
                   nb.status == LampStatusFw::FW_NORMAL_STANDBY);
    nb.settling = false;
  }

  diagA = na;
  diagB = nb;

  // Log serial (throttle 5 detik) untuk verifikasi lapangan
  static unsigned long lastDiagLog = 0;
  if (now - lastDiagLog >= 5000) {
    lastDiagLog = now;
    Serial.printf("[DIAG] A:%s (%.0fW %.0fV) B:%s (%.0fW %.0fV)%s\n",
                  lampStatusName(diagA.status), pA, vA,
                  lampStatusName(diagB.status), pB, vB,
                  (settlingA || settlingB) ? " [settling]" : "");
  }
}

// ════════════════════════════════════════════════════════════════════════════
// OLED Display — layout 7 baris size 1 (permintaan Pak Amin Rodhi)
//   y=0  : Waktu (rata tengah)
//   y=8  : Wifi: online XXms / offline
//   y=16 : Source: PLN / UPS
//   y=24 : Blok A: XXXXW
//   y=32 : Status: Standby / Normal / Gangguan
//   y=40 : Blok B: XXXXW
//   y=48 : Status: Standby / Normal / Gangguan
//   y=56 : sisa 8px
//
// Catatan font: Adafruit_GFX hanya punya size 1 (6×8px) dan size 2 (12×16px).
// Semua baris di sini size 1 — 7×8 = 56px dari 64px.
// ════════════════════════════════════════════════════════════════════════════
void updateOLED() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  // ── Baris 1: Waktu (RTC) — rata tengah ─────────────────────────────────────
  // 8 char × 6px = 48px → x = (128 - 48) / 2 = 40
  DateTime now = rtc.now();
  display.setCursor(40, 0);
  display.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  // ── Baris 2: Status WiFi + latency ping (8.8.8.8) ──────────────────────────
  display.setCursor(0, 8);
  if (inConfigPortal) {
    display.print("Wifi: PORTAL");
  } else if (wifiConnected && lastPingOk) {
    display.printf("Wifi: online %lums", lastPingMs);
  } else {
    display.print("Wifi: offline");
  }

  // ── Baris 3: Sumber daya (PLN / UPS) ───────────────────────────────────────
  display.setCursor(0, 16);
  display.printf("Source: %s", plnPrevState ? "PLN" : "UPS");

  // ── Baris 4-7: Blok A/B — daya + baris status sendiri ──────────────────────
  // Hitung diagnosa dulu (fresh, jalan walau WiFi/Firebase mati).
  updateDiagnostics();

  // Daya efektif: nol kalau tidak bisa dipercaya (PLN mati / basi / invalid).
  // Gate sama dengan yang dipakai evaluator — supaya angka dan label konsisten.
  const bool freshA = (blockA.lastGoodMs != 0) && (millis() - blockA.lastGoodMs < ENERGY_STALE_MS);
  const bool freshB = (blockB.lastGoodMs != 0) && (millis() - blockB.lastGoodMs < ENERGY_STALE_MS);
  const float powerA = (plnPrevState && blockA.valid && freshA) ? blockA.power : 0.0f;
  const float powerB = (plnPrevState && blockB.valid && freshB) ? blockB.power : 0.0f;

  char stA[24], stB[24];
  formatStatusLine(diagA, stA, sizeof(stA));
  formatStatusLine(diagB, stB, sizeof(stB));

  display.setCursor(0, 24);
  display.printf("Blok A: %.0fW", powerA);
  display.setCursor(0, 32);
  display.print(stA);

  display.setCursor(0, 40);
  display.printf("Blok B: %.0fW", powerB);
  display.setCursor(0, 48);
  display.print(stB);

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

  // ── Status validitas data PZEM ─────────────────────────────────────────────
  // PENTING: JANGAN kirim blockA.valid apa adanya.
  // blockA.valid dihitung DI DALAM readPZEM(), yang di-SKIP saat PLN mati.
  // Akibatnya saat PLN mati nilainya tetap true dari pembacaan terakhir (basi).
  // Gabung dengan plnPrevState supaya app tahu data sudah tidak berlaku.
  // Lampu terhubung langsung ke PLN (tanpa UPS), jadi saat PLN mati
  // arus/daya memang 0 — app akan reset tampilan ke 0.
  bool validA = plnPrevState && blockA.valid;
  bool validB = plnPrevState && blockB.valid;

  // Kirim energy data ke /energy/block_a
  FirebaseJson energyJson;
  energyJson.set("voltage", blockA.voltage);
  energyJson.set("current", blockA.current);
  energyJson.set("power", blockA.power);
  energyJson.set("energy", blockA.energy);
  energyJson.set("valid", validA);
  energyJson.set("timestamp", (double)ts);
  Firebase.RTDB.setJSON(&fbdo, "/energy/block_a", &energyJson);

  // Kirim energy data ke /energy/block_b
  FirebaseJson energyJsonB;
  energyJsonB.set("voltage", blockB.voltage);
  energyJsonB.set("current", blockB.current);
  energyJsonB.set("power", blockB.power);
  energyJsonB.set("energy", blockB.energy);
  energyJsonB.set("valid", validB);
  energyJsonB.set("timestamp", (double)ts);
  Firebase.RTDB.setJSON(&fbdo, "/energy/block_b", &energyJsonB);

  // Kirim status PLN + raw RMS ke /energy/z_mpt
  // Pakai plnPrevState (sudah di-debounce) — bukan nilai instan, supaya
  // app tidak flicker saat transisi PLN mati/nyala.
  FirebaseJson zMptJson;
  zMptJson.set("pln", plnPrevState);
  zMptJson.set("voltage_raw", acVoltage);        // RMS ADC mentah (0-4095)
  zMptJson.set("voltage_real", acVoltageReal);   // volt terkalibrasi (RMS * K)
  zMptJson.set("timestamp", (double)ts);
  Firebase.RTDB.setJSON(&fbdo, "/energy/z_mpt", &zMptJson);

  // Kirim status PLN terpisah (untuk notifikasi & status di app)
  FirebaseJson plnJson;
  plnJson.set("available", plnPrevState);
  plnJson.set("source", plnPrevState ? "PLN" : "UPS");
  plnJson.set("timestamp", (double)ts);
  Firebase.RTDB.setJSON(&fbdo, "/energy/pln_status", &plnJson);

  // Heartbeat: device online status (updated tiap 1s bersama energy data)
  Firebase.RTDB.setInt(&fbdo, "/device/heartbeat", (int)ts);

  // Ping status: konektivitas internet device (untuk ditampilkan di app)
  FirebaseJson pingJson;
  pingJson.set("online", lastPingOk);
  pingJson.set("latency_ms", (int)lastPingMs);
  pingJson.set("ssid", WiFi.SSID());
  pingJson.set("timestamp", (double)ts);
  Firebase.RTDB.setJSON(&fbdo, "/device/ping", &pingJson);

  // Note: /lamp/status/{a,b} ditulis oleh aplikasi (updateLampControl)
  // Firmware hanya baca /lamp/control/{a,b} untuk eksekusi perintah
}

// ════════════════════════════════════════════════════════════════════════════
// WiFi Reset Command from Mobile App (via Firebase)
// ════════════════════════════════════════════════════════════════════════════
void checkWifiResetCommand() {
  if (!wifiConnected || !firebaseReady) return;

  // Baca /system/reset_wifi (boolean)
  if (Firebase.RTDB.getBool(&fbdo, "/system/reset_wifi")) {
    if (fbdo.boolData() == true) {
      Serial.println("[RESET] Menerima perintah Reset WiFi!");

      // 1. Kembalikan flag ke false agar tidak looping setelah restart
      Firebase.RTDB.setBool(&fbdo, "/system/reset_wifi", false);

      // 2. Hapus kredensial WiFi di NVS (non-blocking, aman untuk WDT)
      Serial.println("[RESET] Resetting WiFi settings...");
      wm.resetSettings();

      // 3. Restart ESP32 → masuk mode AP (SmartNaga-AP)
      Serial.println("[RESET] Restarting ESP32...");
      ESP.restart();
    }
  }
}

// ════════════════════════════════════════════════════════════════════════════
// Ping Google.com (8.8.8.8) — Connectivity check untuk mobile app
// Pakai ESP-IDF esp_ping (ping_sock.h). Non-blocking: sesi ping berjalan
// di task sendiri, hasil dilaporkan lewat callback.
// ════════════════════════════════════════════════════════════════════════════
// Ping state
esp_ping_handle_t pingHandle = nullptr;

// Callback: balasan ICMP diterima
static void onPingSuccess(esp_ping_handle_t hdl, void *args) {
  uint32_t elapsed = 0;
  esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed, sizeof(elapsed));
  lastPingOk = true;
  lastPingMs = elapsed;
  Serial.printf("[PING] 8.8.8.8 OK, %lu ms\n", (unsigned long)elapsed);
}

// Callback: timeout
static void onPingTimeout(esp_ping_handle_t hdl, void *args) {
  lastPingOk = false;
  lastPingMs = 0;
  Serial.println("[PING] 8.8.8.8 TIMEOUT");
}

// Callback: sesi selesai → hapus sesi agar tidak bocor
static void onPingEnd(esp_ping_handle_t hdl, void *args) {
  esp_ping_delete_session(hdl);
  pingHandle = nullptr;
}

// ════════════════════════════════════════════════════════════════════════════
// Ping Google.com (8.8.8.8) — Connectivity check untuk mobile app
// ════════════════════════════════════════════════════════════════════════════
void pingGoogle() {
  if (inConfigPortal || !wifiConnected) {
    lastPingOk = false;
    lastPingMs = 0;
    return;
  }

  // Jangan tumpuk sesi kalau yang sebelumnya belum selesai
  if (pingHandle != nullptr) return;

  esp_ping_config_t cfg = ESP_PING_DEFAULT_CONFIG();
  cfg.count = 1;                  // 1 paket per siklus
  cfg.timeout_ms = 1000;          // timeout 1 detik
  cfg.interval_ms = 0;
  cfg.data_size = 64;
  cfg.task_stack_size = 4096;
  cfg.task_prio = 1;              // prioritas rendah, jangan ganggu loop
  IP_ADDR4(&cfg.target_addr, 8, 8, 8, 8);  // 8.8.8.8

  esp_ping_callbacks_t cbs = {};
  cbs.on_ping_success = onPingSuccess;
  cbs.on_ping_timeout = onPingTimeout;
  cbs.on_ping_end = onPingEnd;

  if (esp_ping_new_session(&cfg, &cbs, &pingHandle) != ESP_OK) {
    Serial.println("[PING] Gagal membuat sesi ping");
    lastPingOk = false;
    lastPingMs = 0;
    pingHandle = nullptr;
    return;
  }

  esp_ping_start(pingHandle);
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

  // ── Sampling SATU jendela ke buffer ────────────────────────────────────────
  // Semua 600 sampel disimpan dulu, lalu baseline & RMS dihitung dari buffer
  // yang SAMA. Ini mencegah bug baseline dari jendela lama (yang bikin RMS
  // ngawur saat transisi PLN mati/nyala).
  for (int i = 0; i < ZMPT_SAMPLES; i++) {
    zmptBuffer[i] = analogRead(ZMPT_PIN);
    delayMicroseconds(ZMPT_SAMPLE_DELAY_US);
  }

  // ── Baseline = rata-rata (DC offset) ───────────────────────────────────────
  long sumRaw = 0;
  int minVal = 4095, maxVal = 0;
  for (int i = 0; i < ZMPT_SAMPLES; i++) {
    int raw = zmptBuffer[i];
    sumRaw += raw;
    if (raw < minVal) minVal = raw;
    if (raw > maxVal) maxVal = raw;
  }
  float baseline = (float)sumRaw / ZMPT_SAMPLES;

  // ── RMS dari buffer yang sama ──────────────────────────────────────────────
  long sumSqr = 0;
  for (int i = 0; i < ZMPT_SAMPLES; i++) {
    int offset = zmptBuffer[i] - (int)baseline;
    sumSqr += (long)offset * offset;
  }
  float rms = sqrt((float)sumSqr / ZMPT_SAMPLES);
  int peakToPeak = maxVal - minVal;

  // Deteksi valid:
  // 1. RMS < 30 = noise floor (tidak ada tegangan AC)
  // 2. peak-to-peak < 20 = pin floating atau DC saja
  // 3. RMS > 500 = ADC saturate
  bool plnDetected = false;
  if (rms >= 30.0 && peakToPeak >= 20 && rms <= 500.0 && rms > ZMPT_THRESHOLD) {
    plnDetected = true;
  }

  // ── Debounce state PLN ─────────────────────────────────────────────────────
  bool plnAvailable;
  bool firstRead = !plnStateInit;   // simpan sebelum flag diubah

  if (firstRead) {
    // Pembacaan pertama: langsung adopsi state, TANPA alarm
    plnAvailable = plnDetected;
    plnStateInit = true;
    zmptPlnReads = 0;
    zmptUpsReads = 0;
    Serial.printf("[%lu][ZMPT] State awal: %s\n", millis()/1000, plnAvailable ? "PLN" : "UPS");
  } else {
    // Akumulasi pembacaan konsisten
    if (plnDetected) {
      if (zmptPlnReads < PLN_DEBOUNCE_READS) zmptPlnReads++;
      zmptUpsReads = 0;
    } else {
      if (zmptUpsReads < PLN_DEBOUNCE_READS) zmptUpsReads++;
      zmptPlnReads = 0;
    }
    // State hanya berubah setelah debounce terpenuhi
    plnAvailable = plnPrevState;
    if (plnPrevState && zmptUpsReads >= PLN_DEBOUNCE_READS) {
      plnAvailable = false;
    } else if (!plnPrevState && zmptPlnReads >= PLN_DEBOUNCE_READS) {
      plnAvailable = true;
    }
  }

  // RMS mentah untuk threshold; volt terkalibrasi untuk display/Firebase
  acVoltage     = (rms >= 30.0) ? rms : 0.0;
  acVoltageReal = acVoltage * ZMPT_K;

  // Print ke Serial Monitor tiap ~5 detik (throttle)
  // Catatan: Src menampilkan state SETELAH debounce. Saat transisi, baris ini
  // bisa menampilkan state lama — itu normal (print di-throttle 5s, read 100ms).
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 5000) {
    lastPrint = millis();
    const char* src = plnAvailable ? "PLN" : "UPS";
    Serial.printf("[%lu][ZMPT] RMS:%.1f (%.1fV) P2P:%d Base:%.1f Src:%s\n",
                  millis()/1000, rms, acVoltageReal, peakToPeak, baseline, src);
  }

  // Alarm PLN: hanya saat TRANSISI nyata (bukan pembacaan pertama)
  if (!firstRead) {
    if (plnPrevState && !plnAvailable) {
      buzzerOn(2000);
      Serial.printf("[%lu][ALARM] PLN MATI!\n", millis()/1000);
    } else if (!plnPrevState && plnAvailable) {
      Serial.printf("[%lu][ALARM] PLN KEMBALI\n", millis()/1000);
    }
  }
  plnPrevState = plnAvailable;
}