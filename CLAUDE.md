# Project Configuration

## Stack
- **ESP32 firmware**: PlatformIO in `src`
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
# Firmware
cd firmware && pio run -t upload

# Mobile (jika terminal di smartnaga_app, jalankan langsung)
flutter clean && flutter pub get && flutter run

# Git
git add . && git commit -m "feat: <brief>" && git push
```

## Workflow
- Branch: `feature/<task_id>/<brief>`
- PR: 1 owner, 1 peer review

## Wiring (TBD — hardware belum beli)

## Note
- Jika terminal berada di `/Volumes/Data Shared/Project/magang/smartnaga_app/`, cukup tunjukkan command Flutter, jangan eksekusi sendiri. User yang jalankan di terminal-nya.