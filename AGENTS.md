# Project Configuration

## Stack
- **ESP32 firmware**: PlatformIO — root repo ini (`platformio.ini` di root, source di `src/`)
- **Mobile app**: Flutter di `/Volumes/Data Shared/Project/magang/smartnaga_app`
- **Backend**: Firebase Realtime Database (real-time) + Firestore (historical logs)
- **WiFi management**: ESP32 WiFi Manager (OTM - On The Fly Configuration)

## Rules
- Only modify RTDB schema with team consensus
- Linting non-strict; focus on critical bugs
- Fix bugs before PR
- Doc comments / README required

## Commands
```bash
# Firmware (dari root repo ini)
pio run                      # Build
pio run -t upload            # Build + upload
pio device monitor           # Serial monitor
pio lib install "<nama>"     # Tambah library

# Mobile (jika terminal di smartnaga_app, jalankan langsung)
flutter clean && flutter pub get && flutter run

# Git
git add . && git commit -m "feat: <brief>" && git push
```

## Workflow
- Branch: `feature/<task_id>/<brief>`
- PR: 1 owner, 1 peer review
- Test requirement: fix bugs before PR
- Documentation requirement: doc comments / README

## Library Preferences
- Firmware: `pio lib install "<nama>"`
- Flutter: dependency lokal
- Prioritas: perangkat yang benar-benar jalan (equipment that works)

## Wiring
Lihat `WIRING.md` dan `docs/arsitektur-sistem.md`. Pin aktual ada di
`src/main.cpp` bagian `Pin Definition`.

## Note
- Jika terminal berada di `/Volumes/Data Shared/Project/magang/smartnaga_app/`, cukup tunjukkan command Flutter, jangan eksekusi sendiri. User yang jalankan di terminal-nya.
- **Compile & upload firmware dijalankan user sendiri**, bukan oleh agent. Agent hanya edit kode.