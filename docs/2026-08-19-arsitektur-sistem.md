# Arsitektur Sistem — SmartNaga

**Sistem Kontrol Lampu Buah Naga Berbasis IoT**
**Tanggal:** 2026-08-19

---

## 1. Gambaran Umum

```
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
│  │  │ DS3231   │  │ SSD1306  │  │ GPIO13 (A), GPIO4(B) │   │ │
│  │  │ I2C      │  │ I2C      │  │ Active LOW           │   │ │
│  │  └──────────┘  └──────────┘  └──────────────────────┘   │ │
│  └──────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
                              ▲
                              │ WiFi (2.4GHz)
                              │
┌─────────────────────────────────────────────────────────────────┐
│                    DEVICE LAYER                                 │
│  ┌──────────────────┐     ┌──────────────────────┐            │
│  │ Lampu Blok A     │     │ Lampu Blok B         │            │
│  │ (SSR A)          │     │ (SSR B)              │            │
│  └──────────────────┘     └──────────────────────┘            │
│                                                                   │
│  ┌──────────────────────────────────────────┐                   │
│  │  PZEM-004T v3.0 × 2 (energy monitor)     │                   │
│  │  ZMPT101B (AC voltage detection)          │                   │
│  │  DS3231 (RTC timestamp)                   │                   │
│  └──────────────────────────────────────────┘                   │
└─────────────────────────────────────────────────────────────────┘
                              ▲
                              │ WiFi / Mobile Data
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

### 2.1 ESP32 (Microcontroller)
- **WiFi:** Station mode + Config Portal fallback
- **Firebase:** RTDB client dengan auth email/password
- **GPIO:** SSR, buzzer, ZMPT ADC, I2C (OLED + RTC), Serial1/2 (PZEM)
- **RTC:** DS3231 via I2C — timestamp untuk data energy offline

### 2.2 Sensor Energi
| Sensor | Fungsi | Interface | Update Rate | Catatan |
|--------|--------|-----------|-------------|---------|
| PZEM-004T v3.0 (A) | V, I, P, E Blok A | UART (Serial2) | 1 detik | skip saat PLN mati |
| PZEM-004T v3.0 (B) | V, I, P, E Blok B | UART (Serial1) | 1 detik | skip saat PLN mati |
| ZMPT101B | RMS voltage, PLN/UPS | ADC GPIO34 | 100ms | 400 sampel per cycle |
| DS3231 | Timestamp | I2C | NTP sync 1x saat boot | independent dari WiFi |

### 2.3aktuator
| Aktuator | Pin | Fungsi |
|----------|-----|--------|
| SSR Blok A | GPIO13 | ON/OFF lampu Blok A (active LOW) |
| SSR Blok B | GPIO4 | ON/OFF lampu Blok B (active LOW) |
| Buzzer | GPIO23 | Beep toggle (200ms) + PLN alarm (500ms/3s) |

### 2.4 Firebase Realtime Database
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
  └── timestamp: int

/energy/pln_status
  ├── available: bool
  ├── source: string ("PLN"/"UPS")
  └── timestamp: int

/lamp/control/a
  └── bool (true=ON, false=OFF)

/lamp/control/b
  └── bool (true=ON, false=OFF)

/device/heartbeat
  └── int (epoch seconds)
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
     │               │    tiap 1 detik │                │
     │               │                  │ 3. RTDB push  │
     │               │                  │    real-time   │
     │               │                  │                │ 4. Listen
     │               │                  │                │    update UI
```

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
ZMPT101B (GPIO34)
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
ESP32 (tiap 1 detik)
     │ kirim epoch timestamp ke /device/heartbeat
     ▼
Mobile App (poll tiap 2 detik)
     │ cek selisih timestamp sekarang vs last_heartbeat
     ▼
if (selisih > 3 detik)
     │ device offline
     ▼
    Tampilkan "Offline" di UI
```

---

## 4. Timing & Latency

| Operasi | Interval | Latency | Catatan |
|---------|----------|---------|--------|
| PZEM read | 1 detik | 100ms (normal), 7s (timeout) | skip saat PLN mati |
| Firebase send | 1 detik | 500ms-2s (blocking) | 6+ RTDB writes per cycle |
| Firebase command check | 100ms | 100-500ms | debounce 2s per lamp |
| ZMPT read | 100ms | 30ms | 400 sampel ADC, delayMicroseconds |
| OLED update | 1 detik | <10ms | I2C 100kHz |
| Buzzer toggle | on-demand | 200ms (toggle), 500ms (alarm) | millis() safety net |
| PLN detection | 100ms | real-time | threshold 180V RMS |

**End-to-end control latency (App → Lamp):**
- Best case: 100ms (command poll) + 500ms (Firebase write) = **~600ms**
- Worst case: 2s (Firebase lag) + 100ms = **~2.1s**

**End-to-end energy data latency (Sensor → App):**
- Best case: 1s (sensor timer) + 500ms (Firebase) = **~1.5s**
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
    │
    └─ alarm queued ──▶ antri setelah beep aktif selesai
```

---

## 6. Power Management

### PLN → UPS Failover
1. ZMPT101B detects RMS drop (threshold 180V)
2. Transisi PLN→UPS triggers `plnAlarmActive = true`
3. Buzzer alarm beep 500ms setiap 3 detik
4. Mobile app notifikasi via `/energy/pln_status`
5. PZEM read skip (AC-powered, tidak ada power)
6. WiFi + Firebase tetap hidup (via UPS/modem 4G)

### Power Budget
- ESP32: ~240mA @ 5V (WiFi active)
- PZEM × 2: ~50mA @ 5V
- SSR × 2: ~20mA @ 3.3V (signal only, lampu via 220V)
- OLED: ~10mA @ 3.3V
- RTC: ~1mA @ 3.3V (battery backup)
- **Total:** ~320mA @ 5V → ~1.6W

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
