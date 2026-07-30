# PRD: SmartNaga Mobile App

**Sistem Kontrol Pencahayaan Nirkabel Buah Naga — Mobile Application**

---

## 1. Ringkasan Produk

| Aspek              | Detail                                           |
|--------------------|--------------------------------------------------|
| **Nama Project**   | SmartNaga Control                                |
| **Platform**       | Flutter (iOS & Android)                          |
| **Backend**        | Firebase Realtime Database + Firestore           |
| **IoT Device**     | ESP32 + PZEM-004T + SSR Relay                    |
| **Target User**    | Petani dragon fruit, teknisi lapangan            |
| **Core Goal**      | Kontrol lampu 2 blok (A & B) real-time + monitoring energi |

---

## 2. Arsitektur Sistem

```
┌─────────────┐     RTDB (real-time)      ┌─────────────┐
│  Flutter    │ ◄─────────────────────────► │   ESP32     │
│   Mobile    │  /lamp/control/{a,b}        │  Firmware   │
│    App      │  /lamp/status/{a,b}         │             │
└─────────────┘                             └─────────────┘
       │                                           │
       ▼                                           ▼
┌─────────────┐                             ┌─────────────┐
│  Firestore  │                             │   Hardware  │
│ (historical)│                             │  (SSR+PZEM) │
└─────────────┘                             └─────────────┘
```

- **RTDB**: Kontrol lampu real-time, status online/offline
- **Firestore**: Log historis energi, jadwal, audit trail
- **ESP32**: Baca RTDB → kontrol SSR → kirim data PZEM ke RTDB/Firestore

---

## 3. Firebase Schema

### Realtime Database (RTDB)
```json
{
  "lamp": {
    "control": {
      "a": false,      // Target state dari app → ESP32
      "b": false
    },
    "status": {
      "a": { "state": false, "timestamp": 1700000000 },
      "b": { "state": false, "timestamp": 1700000000 }
    }
  },
  "device": {
    "online": true,
    "last_seen": 1700000000,
    "firmware_version": "1.0.0"
  },
  "energy": {
    "a": { "voltage": 220.5, "current": 0.45, "power": 99.2, "energy": 12.3 },
    "b": { "voltage": 220.1, "current": 0.0, "power": 0.0, "energy": 5.1 }
  }
}
```

### Firestore (Historical Logs)
```
logs/{logId}/entries/{entryId}
{
  "lamp_state_a": boolean,
  "lamp_state_b": boolean,
  "power_a_watts": number,
  "power_b_watts": number,
  "voltage_a": number,
  "voltage_b": number,
  "current_a": number,
  "current_b": number,
  "energy_a_kwh": number,
  "energy_b_kwh": number,
  "timestamp": Timestamp,
  "device_id": string
}
```

---

## 4. Fitur Utama (MVP)

### 4.1 Dashboard Kontrol (Home Screen)
- **2 Channel Card** (Blok A & Blok B)
  - Ikon lampu: kuning (hidup) / abu-abu (mati)
  - Badge status: "HIDUP" / "MATI" dengan warna kontras
  - Tombol toggle: Hijau (nyalakan) / Merah (matikan)
  - Timestamp last update per blok

### 4.2 Real-time Sync
- Listener `onValue` ke `/lamp/control/{a,b}`
- Optimistic UI: update lokal → confirm remote
- Auto-reconnect Firebase (exponential backoff)

### 4.3 Monitoring Energi (Opsional MVP)
- Card ringkasan: Voltage, Current, Power, Energy per blok
- Grafik konsumsi harian/mingguan (Flutter charts)

### 4.4 Settings & Device Management
- Device info: firmware version, IP, RSSI
- WiFi re-configuration (link ke WiFiManager portal)
- Threshold alert konfigurasi (opsional)

---

## 5. Hardware Interface (ESP32 Bridge)

### GPIO Mapping (ESP32 DevKit 38-pin)
| Fungsi         | GPIO | Pin Fisik | Catatan                    |
|----------------|------|-----------|----------------------------|
| SSR Blok A     | 13   | D13       | Via LLC level converter    |
| SSR Blok B     | 12   | D12       | Via LLC level converter    |
| PZEM RX        | 16   | RX2       | UART2, 3.3V logic          |
| PZEM TX        | 17   | TX2       | UART2, 3.3V logic          |
| Button Test    | 4    | D4        | INPUT_PULLUP (test_platform) |
| Built-in LED   | 2    | D2        | Active LOW (test_platform) |

### Safety Constraints
- Minimum pulse SSR: 50ms
- Debounce command: 500ms cooldown per channel
- Fail-safe: restart → restore last state from RTDB
- Watchdog: ESP32 reboot jika RTDB disconnect > 60s

---

## 6. User Flow

### Onboarding (First Time)
1. Buka app → "Tambah Perangkat"
2. Scan QR Code di device ESP32 (berisi device_id + WiFi config URL)
3. Join hotspot ESP32 (SSID: `ESP32_Setup_XXXXXXXX`)
4. Masukkan kredensial WiFi rumah → ESP32 connect ke Firebase
5. App verify device online → tampilkan dashboard

### Daily Operation
1. Buka app → auto-connect Firebase (cached auth)
2. Toggle lampu Blok A → kirim ke RTDB `/lamp/control/a = true`
3. ESP32 terima perubahan → nyalakan SSR Blok A
4. ESP32 baca PZEM → update `/energy/a` + `/lamp/status/a`
5. App terima update → refresh UI

---

## 7. Tech Stack

| Layer          | Technology                      | Version       |
|----------------|---------------------------------|---------------|
| Framework      | Flutter                         | 3.24.x        |
| Language       | Dart                            | 3.5.x         |
| State Mgmt     | setState / Provider (future)    | -             |
| Firebase Core  | firebase_core                   | ^4.12.1       |
| RTDB           | firebase_database               | ^12.4.6       |
| Auth           | firebase_auth                   | ^6.5.6        |
| Firestore      | cloud_firestore                 | ^6.5.6        |
| Charts         | fl_chart                        | ^0.68.0       |
| Storage        | firebase_storage                | ^13.4.5       |
| Local Storage  | shared_preferences              | ^2.3.0        |

---

## 8. Non-Functional Requirements

| Kategori           | Requirement                              |
|--------------------|------------------------------------------|
| **Latency**        | UI update < 500ms setelah toggle         |
| **Reliability**    | Auto-reconnect < 5s setelah disconnect   |
| **Offline**        | Cache last state (shared_preferences)    |
| **Security**       | Firebase Rules: auth required, per-user  |
| **Battery**        | Background fetch < 1%/jam                |
| **Accessibility**  | Support VoiceOver / TalkBack             |

---

## 9. Testing Strategy

| Level          | Tools                    | Coverage Target |
|----------------|--------------------------|-----------------|
| Unit           | flutter_test             | ≥ 80%           |
| Widget         | flutter_test             | Key flows       |
| Integration    | firebase_emulator        | Critical paths  |
| E2E Hardware   | ESP32 test_platform.cpp  | Full chain      |

---

## 10. Roadmap

### Phase 1 — MVP (Current)
- [x] 2-channel control UI
- [x] RTDB real-time sync
- [x] Firebase Auth integration
- [ ] Energy monitoring display
- [ ] Offline cache

### Phase 2 — Enhanced
- [ ] Scheduling / Timer
- [ ] Push notifications (FCM)
- [ ] Multi-device support
- [ ] Historical charts (Firestore)

### Phase 3 — Production
- [ ] Multi-user / Role-based access
- [ ] OTA firmware update from app
- [ ] Analytics dashboard
- [ ] Export data (CSV/PDF)

---

## 11. Referensi Kode Terkait

| File                                 | Deskripsi                        |
|--------------------------------------|----------------------------------|
| `lib/main.dart`                      | Entry point + HomeScreen         |
| `lib/firebase_options.dart`          | Firebase config (generated)      |
| `firmware/src/main.cpp`              | Production firmware (PZEM+SSR)   |
| `firmware/src/test_platform.cpp`     | Test firmware (WiFi+Firebase+LED)|
| `firmware/platformio.ini`            | PIO environments                 |
| `firebase/rtdb-sample.json`          | RTDB schema example              |

---

## 12. Catatan Tim

- **Credential Management**: `config.h` di `.gitignore`, developer copy dari `config.h.example`
- **Branch Strategy**: `feature/<task_id>/<brief>`, PR butuh 1 owner + 1 peer review
- **Deploy**: `flutter build apk/ipa` → TestFlight / Play Console Internal

---

*Dokumen ini living document. Update seiring development.*