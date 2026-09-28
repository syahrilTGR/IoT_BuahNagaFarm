# Arsitektur Sistem — SmartNaga

**Sistem Kontrol Lampu Buah Naga Berbasis IoT**
**Tanggal:** 2026-08-19

---

## 1. Gambaran Umum

```
┌─────────────────────────────────────────────────────────────────┐
│              ARUS DAYA AC — JALUR LAMPU (PLN saja)              │
│                                                                   │
│   PLN 220V ── [ZMPT101B] ──┬── SSR_A ── Lampu A                │
│                 (clip-on)  │                                    │
│                            └── SSR_B ── Lampu B                │
│                                                                   │
│   ⚠️ Tanpa UPS — lampu mati total saat PLN mati                 │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼ GND/Neutral
┌─────────────────────────────────────────────────────────────────┐
│           ARUS DAYA DC — JALUR KONTROL (via UPS)                │
│                                                                   │
│   UPS AC → PS 12V → Stepdown 5V → ESP32 + Sensor + Modem 4G    │
│   (ESP32 & sensor tetap hidup saat PLN mati)                    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼ WiFi (2.4GHz)
┌─────────────────────────────────────────────────────────────────┐
│                         CLOUD LAYER                              │
│                     Firebase Realtime DB                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐  │
│  │ /lamp/       │  │ /energy/     │  │ /device/             │  │
│  │ control/{a,b}│  │ block_a      │  │ heartbeat            │  │
│  │ (kontrol)    │  │ block_b      │  │ (online status)      │  │
│  │              │  │ z_mpt        │  │                      │  │
│  │              │  │ pln_status   │  │                      │  │
│  └──────────────┘  └──────────────┘  └──────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              ▲
                              │ HTTPS/WebSocket
                              │
┌─────────────────────────────────────────────────────────────────┐
│                    EDGE LAYER — ESP32                            │
│                                                                   │
│  ┌─────────────┐   ┌──────────────┐   ┌─────────────────────┐  │
│  │ PZEM-004T A │   │  PZEM-004T B │   │  ZMPT101B           │  │
│  │ (Blok A)    │   │  (Blok B)    │   │  (AC Voltage)       │  │
│  │ Serial2     │   │  Serial1     │   │  GPIO34/ADC1_CH6    │  │
│  │ GPIO33/32   │   │  GPIO25/26   │   │  RMS raw ADC        │  │
│  └──────┬──────┘   └──────┬──────┘   └──────────┬──────────┘  │
│         │                  │                      │             │
│  ┌──────▼──────────────────▼──────────────────────▼──────────┐ │
│  │                    ESP32 DevKit                            │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────────────────┐   │ │
│  │  │ WiFi     │  │ Firebase │  │ Buzzer               │   │ │
│  │  │ Manager  │◄─│ Client   │  │ GPIO23               │   │ │
│  │  │ (STA/AP) │  │ (RTDB)   │  │ PLN alarm            │   │ │
│  │  └──────────┘  └──────────┘  └──────────────────────┘   │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────────────────┐   │ │
│  │  │ RTC      │  │ OLED     │  │ SSR A/B              │   │ │
│  │  │ DS3231   │  │ SSD1306  │  │ GPIO14 (A), GPIO13(B) │   │ │
│  │  │ I2C      │  │ I2C      │  │ Active LOW           │   │ │
│  │  └──────────┘  └──────────┘  └──────────────────────┘   │ │
│  └──────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
                              ▲
                              │ WiFi (2.4GHz)
                              │
┌─────────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                             │
│               Aplikasi Mobile (Flutter)                          │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │ • Monitor: V, I, P, E per blok                           │   │
│  │ • Kontrol: ON/OFF lampu A + B                             │   │
│  │ • Notifikasi: PLN mati, anomali daya                       │   │
│  │ • Status: online/offline device                            │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. Komponen Sistem

### 2.1 Power Supply Chain

```
AC Input — JALUR LAMPU (langsung PLN, TANPA UPS)
├── PLN 220V (Live + Neutral)
│        ├── ZMPT101B (clip-on) → ESP32 ADC (deteksi PLN ada/mati)
│        ├── SSR_A (Live switched) → Lampu Blok A
│        └── SSR_B (Live switched) → Lampu Blok B
│
│   ⚠️ Lampu mati total saat PLN mati — tidak ada backup.
│      ZMPT GND ── ESP32 GND (shared reference)

AC Input — JALUR KONTROL (via UPS)
└── UPS AC Output (220V)
         │
         ▼
    Power Supply 12V → Stepdown 5V
         ├── ESP32 VIN (5V) + 3V3 (3.3V output)
         ├── PZEM-A VCC (3.3V dari ESP32)
         ├── PZEM-B VCC (3.3V dari ESP32)
         ├── ZMPT VCC (5V)
         └── Peripheral 3.3V (OLED, RTC, Buzzer) dari ESP32 3V3

    Modem 4G: Adapter cas 5V → UPS
    UPS hanya menopang ESP32 + sensor + modem 4G.
```

### 2.2 ESP32 (Microcontroller)
- **WiFi:** Station mode + Config Portal fallback (modem 4G hotspot)
- **Firebase:** RTDB client dengan auth email/password
- **GPIO:** SSR, buzzer, ZMPT ADC, I2C (OLED + RTC), Serial1/2 (PZEM)
- **RTC:** DS3231 via I2C — timestamp untuk data energy offline

### 2.3 Sensor Energi
| Sensor | Fungsi | Interface | Update Rate | Catatan |
|--------|--------|-----------|-------------|---------|
| PZEM-004T v3.0 (A) | V, I, P, E Blok A | UART (Serial2) | 2.5 detik | skip saat PLN mati, 3.3V |
| PZEM-004T v3.0 (B) | V, I, P, E Blok B | UART (Serial1) | 2.5 detik | skip saat PLN mati, 3.3V |
| ZMPT101B | RMS voltage, PLN/UPS | ADC GPIO34 | 100ms | 400 sampel per cycle |
| DS3231 | Timestamp | I2C | NTP sync 1x saat boot | independent dari WiFi |

### 2.4 Aktuator
| Aktuator | Pin | Fungsi |
|----------|-----|--------|
| SSR Blok A | GPIO14 | ON/OFF lampu Blok A (active LOW) |
| SSR Blok B | GPIO13 | ON/OFF lampu Blok B (active LOW) |
| Buzzer | GPIO23 | Beep toggle (200ms) + PLN alarm (500ms/3s) |

### 2.5 Firebase Realtime Database
```
/energy/block_a
  ├── voltage: float (V)
  ├── current: float (A)
  ├── power: float (W)
  ├── energy: float (kWh)
  └── timestamp: int (epoch)

/energy/block_b
  └── (sama seperti block_a)

/energy/z_mpt
  ├── pln: bool (PLN available)
  ├── voltage_raw: float (RMS ADC)
  ├── voltage_real: float (volt terkalibrasi, RMS × 0.667)
  └── timestamp: int (epoch)

/energy/pln_status
  ├── available: bool
  ├── source: string ("PLN"/"UPS")
  └── timestamp: int (epoch)

/lamp/control/a
  └── bool (true=ON, false=OFF)

/lamp/control/b
  └── bool (true=ON, false=OFF)

/device/heartbeat
  └── int (epoch seconds)

/device/ping
  ├── online: bool (ping 8.8.8.8 berhasil)
  ├── latency_ms: int (waktu round-trip)
  └── timestamp: int (epoch)

/system/reset_wifi
  └── bool (true = perintah reset WiFi dari app)
```

---

## 3. Alur Komunikasi

### 3.1 Data Flow (Sensor → Cloud → App)

```
┌──────────┐    ┌──────────┐    ┌───────────┐    ┌──────────┐
│ PZEM-A/B │───▶│  ESP32   │───▶│ Firebase  │───▶│  Mobile  │
│ ZMPT101B │    │ (loop)   │    │   RTDB    │    │   App    │
└──────────┘    └──────────┘    └───────────┘    └──────────┘
     │               │                  │                │
     │ 1. Baca V/I/P │                  │                │
     │               │ 2. Kirim JSON   │                │
     │               │    tiap 2.5 detik │              │
     │               │                  │ 3. RTDB push  │
     │               │                  │    real-time   │
     │               │                  │                │ 4. Listen
     │               │                  │                │    update UI
```

Termasuk: energy (PZEM A/B), ZMPT, PLN status, heartbeat, **ping (8.8.8.8)**.

### 3.2 Control Flow (App → Cloud → ESP32 → Aktuator)

```
┌──────────┐    ┌──────────┐    ┌───────────┐    ┌──────────┐
│  Mobile  │───▶│ Firebase │───▶│  ESP32    │───▶│   SSR    │
│   App    │    │   RTDB   │    │ (polling) │    │  ON/OFF  │
└──────────┘    └──────────┘    └───────────┘    └──────────┘
     │               │                  │                │
     │ 1. Toggle A   │                  │                │
     │               │ 2. Write         │                │
     │               │    /lamp/control/a│               │
     │               │                  │ 3. Cek tiap    │
     │               │                  │    100ms        │
     │               │                  │                │ 4. GPIO write
     │               │                  │                │    SSR ON/OFF
     │               │                  │                │ 5. Buzzer beep
```

### 3.3 PLN Detection Flow

```
ZMPT101B (clip-on pada stopkontak)
     │ analogRead 400 sampel (RMS)
     ▼
readZMPT()
     │ hitung RMS, peak-to-peak, baseline
     ▼
if (RMS > 180V threshold)
     │
     ├─▶ PLN available → update OLED "PLN"
     │               → kirim pln_status=true ke Firebase
     │
     └─▶ PLN unavailable → update OLED "UPS"
                       → kirim pln_status=false
                       → trigger PLN MATI alarm (buzzer 500ms/3s)
```

### 3.4 Heartbeat Flow

```
ESP32 (tiap 2.5 detik)
     │ kirim epoch timestamp ke /device/heartbeat
     ▼
Mobile App (poll tiap 3 detik)
     │ cek selisih timestamp sekarang vs last_heartbeat
     ▼
if (selisih > 5 detik)
     │ device offline
     ▼
    Tampilkan "Offline" di UI
```

---

## 4. Timing & Latency

| Operasi | Interval | Latency | Catatan |
|---------|----------|---------|--------|
| PZEM read | 2.5 detik | 100ms (normal), 7s (timeout) | skip saat PLN mati |
| Firebase send | 2.5 detik | 500ms-2s (blocking) | 7+ RTDB writes per cycle |
| Firebase command check | 100ms | 100-500ms | debounce 2s per lamp |
| ZMPT read | 100ms | 30ms | 600 sampel ADC, delayMicroseconds |
| OLED update | 1 detik | <10ms | I2C 100kHz |
| Buzzer toggle | on-demand | 200ms (toggle), 2000ms (alarm) | millis() safety net |
| PLN detection | 100ms | real-time | threshold 150V RMS |
| **Ping 8.8.8.8** | **3 detik** | **~20-200ms** | **non-blocking (esp_ping task)** |

**End-to-end control latency (App → Lamp):**
- Best case: 100ms (command poll) + 500ms (Firebase write) = **~600ms**
- Worst case: 2s (Firebase lag) + 100ms = **~2.1s**

**End-to-end energy data latency (Sensor → App):**
- Best case: 2.5s (sensor timer) + 500ms (Firebase) = **~3s**
- Worst case: 7s (PZEM timeout) + 2s = **~9s**

---

## 5. State Management

### ESP32 State Machine (WiFi + Firebase)

```
SYS_WIFI_INIT
    │
    ▼
SYS_WIFI_CONNECTING → WiFi.begin()
    │
    ├─ Connected ──▶ SYS_FIREBASE_INIT
    │
    └─ Timeout 30s ──▶ SYS_WIFI_PORTAL (Config Portal AP)
                           │
                           ├─ Configured ──▶ SYS_FIREBASE_INIT
                           └─ Failed ──▶ ESP.restart()

SYS_FIREBASE_INIT
    │ Firebase.begin() + configTime(NTP)
    ▼
SYS_FIREBASE_CHECK → tunggu Firebase.ready() (max 20s)
    │
    ├─ Ready ──▶ SYS_READY (normal operation)
    │               ├─ sync default lamp state dari RTDB
    │               ├─ syncRTC() dari NTP
    │               └─ start loop normal
    │
    └─ Timeout ──▶ SYS_READY (limited mode, tanpa Firebase)
```

### Buzzer State Machine

```
IDLE (buzzerActive=false)
    │
    ├─ toggle A/B ──▶ BEEP_200 (200ms) ──▶ IDLE
    │
    ├─ PLN MATI ──▶ BEEP_500 (500ms) ──▶ WAIT_3s
    │                       │
    │                       ▼
    │                  WAIT_3s (silence 3s)
    │                       │
    │                       ├─ PLN still off ──▶ BEEP_500
    │                       │
    │                       └─ PLN kembali ──▶ IDLE
```

---

## 6. Power Management

### AC Power Distribution

```
AC INPUT (220V 50Hz)
│
├── JALUR LAMPU — PLN langsung, tanpa UPS
│   ├── PLN Live ── [ZMPT101B clip-on] ──┬── SSR_A ── Lampu A
│   │                │                   └── SSR_B ── Lampu B
│   │                ├── ZMPT VCC ← 5V (dari stepdown)
│   │                ├── ZMPT GND ← ESP32 GND
│   │                └── ZMPT Output → GPIO34
│   │
│   └── Neutral (shared) ── semua beban
│       ⚠️ Lampu mati total saat PLN mati
│
└── JALUR KONTROL — via UPS
    └── UPS AC Output → PS 12V → Stepdown 5V → ESP32 + sensor
```

### DC Power Distribution

```
UPS AC Output
    │
    ├── Adapter Cas 5V → Modem 4G Hotspot
    │   (WiFi source selalu ON saat UPS aktif)
    │
    └── Power Supply 12V
        │
        ├── 12V rail
        │   └── Stepdown 12V → 5V (buck converter)
        │       ├── 5V rail
        │       │   ├── ESP32 VIN (5V input)
        │       │   ├── ZMPT101B VCC (5V)
        │       │   └── 3.3V rail (dari ESP32 3V3 pin)
        │       │           ├── PZEM-A VCC (3.3V)
        │       │           ├── PZEM-B VCC (3.3V)
        │       │           ├── OLED VCC
        │       │           ├── RTC VCC
        │       │           ├── Buzzer VCC
        │       │           └── SSR LED+ (langsung dari GPIO)
        │       │
        │       └── GND rail (shared)
        │           ├── ESP32 GND
        │           ├── PZEM-A GND
        │           ├── PZEM-B GND
        │           ├── ZMPT GND
        │           ├── OLED GND
        │           ├── RTC GND
        │           └── Buzzer GND
```

### Power Budget

| Komponen | Konsumsi | Sumber |
|----------|----------|--------|
| ESP32 (WiFi active) | ~240mA @ 5V | Stepdown 5V |
| PZEM-A | ~25mA @ 3.3V | 3.3V rail |
| PZEM-B | ~25mA @ 3.3V | 3.3V rail |
| OLED SSD1306 | ~10mA @ 3.3V | 3.3V rail |
| RTC DS3231 | ~1mA @ 3.3V | 3.3V rail + battery |
| Buzzer (active) | ~30mA @ 3.3V | 3.3V rail |
| SSR A/B (signal only) | ~20mA @ 3.3V | 3.3V rail |
| Modem 4G | ~500mA @ 5V | Adapter cas UPS |
| Lampu (via SSR, 220V) | ~15W @ 220V | AC PLN saja |
| **Total DC** | **~850mA @ 5V** | ~4.25W |
| **Total AC** | **~30W** (2 lampu) | PLN saja (tanpa UPS) |

### PLN Mati — Perilaku Sistem

**Penting: lampu TIDAK punya backup UPS.** Lampu terhubung langsung ke PLN
via SSR. Saat PLN mati, lampu mati total dan tidak bisa dinyalakan dari app.

Yang tetap hidup: **ESP32 + sensor + modem 4G** (ditopang UPS).

```
PLN 220V ON:
├── ZMPT101B RMS > 150 → PLN available
├── Lampu A/B bisa ON via SSR (sumber: PLN)
├── ESP32 powered via UPS → stepdown 5V → 3.3V
├── Modem 4G via adapter cas 5V → UPS
└── WiFi + Firebase connect ke 4G hotspot

PLN 220V OFF:
├── ZMPT101B RMS < 150 → UPS state
├── ⚠️ Lampu A/B MATI TOTAL — tidak ada backup, tidak bisa dinyalakan dari app
├── ⚠️ PZEM tidak bisa ukur (sumber AC-nya hilang)
├── ESP32 tetap hidup (via UPS → stepdown 5V)
├── Modem 4G tetap hidup (via adapter cas 5V → UPS)
├── WiFi tetap connect ke 4G hotspot
├── Buzzer alarm beep panjang 2 detik (sekali, saat transisi)
├── PZEM read skip (AC-powered, tidak ada power)
└── Firebase tetap menerima data dari ESP32 (status + ZMPT)
```

**Konsekuensi untuk data energi:**
Karena lampu mati saat PLN mati, arus & daya yang benar adalah **0**.
Data PZEM yang terakhir terbaca (sebelum PLN mati) adalah **basi** dan harus
dianggap tidak valid — bukan nilai yang masih berlaku.

**Flow data saat PLN mati:**
```
ESP32 → WiFi 4G hotspot → Firebase → Mobile App
         (via UPS power)
```

**Alasan PZEM skip saat PLN mati:**
- PZEM 004T v3.0 powered dari AC yang diukur
- Saat PLN mati, PZEM tidak punya power → tidak ada response
- Modbus read timeout ~7 detik per attempt
- Skip untuk mencegah blocking loop selama alarm buzzer aktif

---

## 7. Security

| Layer | Mechanism |
|-------|-----------|
| WiFi | WPA2-PSK (Config Portal AP mode for initial setup) |
| Firebase Auth | Email + password (custom token not used) |
| RTDB Rules | `auth != null` (authenticated users only) |
| Device ID | Unique MAC address in Config Portal AP name |

---

## 8. Failure Modes

| Failure | Detection | Recovery |
|---------|-----------|----------|
| WiFi disconnect | `WiFi.status() != WL_CONNECTED` | Auto reconnect, Config Portal fallback |
| Firebase timeout | 20s `SYS_FIREBASE_CHECK` | Retry later, limited mode |
| PZEM no response | NaN voltage/current | Skip read, retry next cycle |
| ZMPT noise | P2P < 20 or RMS < 30 | Mark as UPS, skip alarm |
| RTC battery dead | NTP fallback to compile time | Set waktu dari NTP setiap boot |
| PLN mati | ZMPT RMS < 150 | Lampu mati total (tanpa backup); ESP32 & modem tetap hidup via UPS |

---

## 9. Dependencies

```
PlatformIO (ESP32):
  - PZEM-004T-v30 @ 1.1.2
  - WiFiManager @ 2.0.17
  - Firebase Arduino Client @ 4.4.17
  - Adafruit SSD1306 @ 2.5.17
  - Adafruit RTClib @ 2.1.4
  - ArduinoJson @ 7.4.3
```

---

## 10. Future Improvements

1. **RTOS migration** (plan di `docs/2026-08-19-rtos-plan.md`) — untuk isolate buzzer dari blocking calls
2. **OTA update** — update firmware via Firebase/OTA
3. **Energy logging** — Firestore untuk historical data (bukan cuma RTDB)
4. **Scheduler** — on/off otomatis berdasarkan waktu RTC
5. **Multi-tenant** — support lebih dari 1 device per Firebase project
