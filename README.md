# Sistem Kontrol Pencahayaan Nirkabel Buah Naga

Prototipe IoT untuk monitoring & kontrol pencahayaan budidaya buah naga dengan monitoring konsumsi energi listrik real-time.

## Struktur Project

```
kontrol-lampu-buah-naga/
├── docs/                    # Dokumentasi & proposal
│   ├── Proposal.md
│   └── Proposal.pdf
├── src/                     # ESP32 code (PlatformIO)
├── firebase/                # Firebase config (rules, indexes, schema)
├── smartnaga_app/           # Mobile app (Flutter)
├── hardware/                # KiCad schematics & PCB
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
| Hardware Design | KiCad |

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

### Hardware (KiCad)
Open `hardware/kontrol-lampu.kicad_pro` di KiCad 8+

## Roadmap (dari Proposal)

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