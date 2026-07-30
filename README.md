# Sistem Kontrol Pencahayaan Nirkabel Buah Naga

Prototipe IoT untuk monitoring & kontrol pencahayaan budidaya buah naga dengan monitoring konsumsi energi listrik real-time.

## Struktur Project

```
kontrol-lampu-buah-naga/
├── docs/                    # Dokumentasi & plan
│   ├── block-diagram.md
│   └── plan.md
├── src/                     # ESP32 code (PlatformIO)
│   ├── main.cpp             # Production firmware (PZEM + SSR + WiFiManager + WiFi)
│   ├── test_platform.cpp    # Minimal test firmware (LED + button + Firebase RTDB)
│   └── config.h.example     # Template credential config (copy to config.h, gitignored)
├── firebase/                # Firebase config (rules, indexes, schema)
│   └── rtdb-sample.json
├── smartnaga_app/           # Mobile app (Flutter)
├── hardware/                # Hardware design (Fritzing, EasyEDA)
│   ├── components.md
│   └── skema rangkaian.fzz
└── images/                  # Diagram, foto prototype
```

## Stack Teknologi

| Layer | Teknologi |
|-------|-----------|
| MCU | ESP32 (Arduino/PlatformIO) |
| Sensor Energi | PZEM-004T v3.0 (2x: Blok A & B) |
| Solid State Relay | 2-channel (lampu Blok A & B) |
| Connectivity | USB Modem LTE → Internet |
| Manajemen Koneksi | ESP32 WiFi Manager (OTM - On The Fly Configuration) |
| Backend | Firebase Realtime Database (real-time) + Firestore (historical logs) |
| Mobile App | Flutter (recommended) / React Native |
| Hardware Design | Fritzing (prototyping), EasyEDA (PCB production) |

##Rencana Fitur tambahan
- Data historis otomatis dipindahkan dari RTDB ke **Firestore** menggunakan Firebase Cloud Function
- Backup data & riwayat konsumsi energi di Firestore untuk analisis lama
- Ekspor manual dari dashboard aplikasi mobile (CSV/PDF)

## Quick Start

### Firmware (ESP32)
```bash
cd firmware
pio run -t upload
```

### Mobile App (Flutter)
```bash
cd /Volumes/Data Shared/Project/magang/smartnaga_app
flutter pub get
flutter run
```

### Hardware (Fritzing / EasyEDA)
Open `hardware/skema rangkaian.fzz` di Fritzing untuk prototyping, atau import ke EasyEDA untuk PCB production.

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