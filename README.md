# Sistem Kontrol Pencahayaan Nirkabel Buah Naga

Prototipe IoT untuk monitoring & kontrol pencahayaan budidaya buah naga dengan monitoring konsumsi energi listrik real-time.

## Struktur Project

```
kontrol-lampu-buah-naga/
├── docs/                    # Dokumentasi, PRD, & arsitektur sistem
│   ├── 2026-08-19-arsitektur-sistem.md
│   ├── 2026-09-28-decision-tree-esp32-design.md
│   ├── block-diagram.md
│   └── plan.md
├── src/                     # ESP32 code (PlatformIO)
│   ├── main.cpp             # Production firmware (PZEM + SSR + OLED + ZMPT + RTC + Buzzer + Firebase)
│   ├── test_platform.cpp    # Test firmware (WiFi + Firebase + RTClib)
│   ├── test_zmpt.cpp        # Diagnostik hardware sensor tegangan ZMPT101B
│   ├── config.h             # Credential & endpoint config (gitignored)
│   └── config.h.example     # Template credential config
├── firebase/                # Firebase config (rules, indexes, schema)
│   ├── database.rules.json  # Security rules Firebase RTDB
│   ├── firestore.indexes.json # Composite indexes Firestore
│   └── rtdb-sample.json     # Contoh struktur data RTDB
├── hardware/                # Hardware design (Fritzing, EasyEDA)
│   ├── components.md
│   └── skema rangkaian.fzz
├── images/                  # Diagram, foto prototype
├── platformio.ini           # Konfigurasi PlatformIO multi-environment
├── WIRING.md                # Panduan pengkabelan dan pinout ESP32
├── FIREBASE_RULES.md        # Dokumentasi schema & rules Firebase
├── PZEM-004T.md             # Referensi teknis sensor PZEM-004T
└── TROUBLESHOOTING.md       # Panduan penanganan kendala teknis
```
> **Catatan:** Aplikasi mobile (**SmartNaga App**) berada pada repositori/direktori terpisah di:
> `/Volumes/Data Shared/Project/magang/smartnaga_app`

## Stack Teknologi

| Layer | Komponen / Teknologi | Keterangan |
|-------|----------------------|------------|
| MCU | ESP32 DevKit (Arduino / PlatformIO) | Kontroler utama dual-core, konektivitas WiFi |
| Sensor Energi | PZEM-004T v3.0 (2x: Blok A & B) | Monitoring tegangan, arus, daya, & akumulasi energi |
| Deteksi Sumber Daya | ZMPT101B Voltage Sensor | Deteksi real-time suplai listrik PLN vs UPS |
| Real-Time Clock | RTC DS3231 (I2C) | Jam real-time presisi tinggi untuk operasional & jadwal offline |
| Display Lokal | OLED SSD1306 0.96" 128x64 (I2C) | Tampilan lokal waktu, WiFi & ping latency, daya, status anomali |
| Audio Alert | Active Buzzer | Alarm suara pemadaman PLN & peringatan anomali beban |
| Aktuator Lampu | Solid State Relay (SSR) 2-channel | Pengendalian lampu Blok A & Blok B (AC zero-cross) |
| Konektivitas | WiFi / USB Modem LTE (Hotspot) | Penghubung ESP32 ke internet |
| Manajemen Koneksi | ESP32 WiFi Manager | OTM (On-The-Fly Configuration via AP Web Portal) |
| Cloud Backend | Firebase RTDB + Firestore | RTDB untuk telemetri/kontrol real-time, Firestore untuk sesi histori |
| Mobile App | Flutter (`smartnaga_app`) | Monitoring, kontrol manual/jadwal, konfigurasi lampu, ekspor data |
| Hardware Design | Fritzing & EasyEDA | Prototyping skema rangkaian & desain PCB |

## Rencana Fitur Tambahan
- Data historis otomatis dipindahkan dari RTDB ke **Firestore** menggunakan Firebase Cloud Function / Session Service
- Backup data & riwayat konsumsi energi di Firestore untuk analisis jangka panjang
- Ekspor manual dari dashboard aplikasi mobile (CSV/Spreadsheet/PDF)

## Quick Start

### Firmware (ESP32)
Jalankan perintah PlatformIO langsung dari root direktori repo ini:
```bash
# Build firmware production (esp32dev)
pio run -e esp32dev

# Build & upload ke board ESP32
pio run -t upload -e esp32dev

# Monitor serial output (115200 baud)
pio device monitor -e esp32dev

# Upload & jalankan diagnostik sensor ZMPT (opsional)
pio run -t upload -e test_zmpt && pio device monitor -e test_zmpt
```

### Mobile App (Flutter)
```bash
cd "/Volumes/Data Shared/Project/magang/smartnaga_app"
flutter pub get
flutter run
```

### Hardware (Fritzing / EasyEDA)
Open `hardware/skema rangkaian.fzz` di Fritzing untuk prototyping, atau import ke EasyEDA untuk PCB production.
Detail pinout dan pengkabelan lengkap dapat dilihat pada `WIRING.md` serta `src/main.cpp`.

## Roadmap

| Tahun | Fokus |
|-------|-------|
| 2026 | Prototipe Node Tunggal (TKT 3) |
| 2027 | Multi-node WSN |
| 2028 | Integrasi PLTS + AI Edge |
| 2029 | Diagnostik Cerdas (AI) |
| 2030 | Smart Energy Management Platform |

## Luaran Target

- ✅ Prototipe sistem (TKT 3)
- 📝 Jurnal SINTA 3 (TELKA)
- 📝 Hak Cipta kode & desain sistem