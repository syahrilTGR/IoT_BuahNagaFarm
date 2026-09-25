pelajari:
- millis()
- rangkaian pzem ke esp32

note:
- yg ke SSR itu yg line phase
- esp32 ada berapa uart
- nyalanya lampu tu ngga semalaman
- ssr udah bisa, dari esp langsung ssr, sebelumnya pake LLC, gabisa, kyk nya LLC nya rusak, di sisi 5v ngga mau ke state low

di esp
- daya di oled buat atas bawah aja

di App
- [x] tambah estimasi rup
- [x] coba ekspor historis dari firestore ke spreadsheet/csv
- [x] di setting, tambahkan inputan daya dan jumlah lampu default, yang nantinya disimpan ke firebase dan digunakan sebagai acuan kondisi normal atau tidak
- [x] notif lampu padam (dari ZMPT), nanti source nya berubah jadi UPS

di App — PERLU DIPERBAIKI (temuan audit firmware 2026-09-25)
- [x] **`_zmptVoltage` sebenarnya berisi tegangan PZEM Blok A, bukan ZMPT.**
      Di `home_screen.dart` ada DUA listener menulis ke `_zmptVoltage`:
        1. `_plnRef.child('voltage')` (L162) → `/energy/pln_status/voltage` **TIDAK ADA** → listener mati, tidak pernah fire
        2. `_energyRefA.child('voltage')` (L172) → `/energy/block_a/voltage` → ini yang jalan (PZEM, ~238V)
      Jadi tampilan tegangan "kebetulan" benar angkanya, tapi sumbernya PZEM Blok A — bukan ZMPT.
      → Hapus listener mati (L162-170), dan putuskan mau pakai yang mana:
        - **PZEM Blok A** (tegangan di sisi beban) — kalau mau tetap seperti sekarang, rename variabelnya
        - **ZMPT** (`/energy/z_mpt/voltage_real`, tegangan di jalur PLN sebelum stopkontak) — kalau mau bedakan
- [x] **Field baru `voltage_real`** — firmware kirim volt terkalibrasi (RMS × 0.667) ke
      `/energy/z_mpt/voltage_real`. Ini satu-satunya sumber tegangan dari ZMPT.
- [x] **`voltage_raw` = RMS ADC mentah, BUKAN volt** — nilainya ~0-500 (contoh 356), bukan ~238.
      Jangan ditampilkan sebagai volt.
- [x] **Heartbeat staleness tidak konsisten** — `control_screen.dart` pakai 10s, `home_screen.dart`
      pakai 15s. Firmware kirim tiap 2.5s. Samakan (usul 10s = 4 beat terlewat).
- [x] **Status PLN bisa flicker saat transisi** — firmware kirim `/energy/pln_status/available`
      dari nilai INSTAN, bukan yang sudah di-debounce. Saat PLN baru mati/nyala, app bisa
      sempat menampilkan status salah ~300ms. Pertimbangkan smoothing di app (debounce 2-3s)
      atau perbaiki di firmware (pakai `plnPrevState`).
- [x] **BUG: arus & daya stuck di nilai terakhir saat PLN mati** (dilaporkan 2026-09-25)
      Gejala: PLN normal + lampu nyala → terukur arus & daya. Lalu PLN mati →
      nilai arus/daya **tidak reset ke 0**, tetap menampilkan angka terakhir.

      **Akar masalah ADA DI FIRMWARE, bukan app.** Alur:
        1. `src/main.cpp:341` — `readPZEM()` di-SKIP total saat PLN mati:
           `if (now - timerPZEM.prev >= timerPZEM.interval && plnPrevState)`
        2. `blockA` / `blockB` jadi variabel basi (nilai terakhir sebelum PLN mati)
        3. `sendToFirebase()` tetap kirim `blockA.power` / `blockA.current` yang basi
           itu ke `/energy/block_{a,b}` setiap 2.5s
        4. App menerima nilai valid → tidak ada cara tahu itu sudah tidak berlaku

      **Fix firmware (pilih salah satu):**
        - **A (disarankan):** saat PLN mati, tulis nol ke `/energy/block_{a,b}`
          (voltage/current/power = 0), karena PZEM memang tidak bisa mengukur
        - **B:** kirim flag `valid: false` di JSON, app yang menampilkan "—" / 0
        - **C:** keduanya — nol-kan nilai + kirim flag

      **Catatan penting:** PZEM mengukur sisi BEBAN (setelah relay). Saat PLN mati,
      kalau lampu masih jalan dari UPS, arus sebenarnya BUKAN nol — PZEM hanya tidak
      bisa membacanya karena VCC 3.3V brownout (lihat item hardware PZEM VCC 5V).
      Jadi fix yang benar jangka panjang: perbaiki VCC PZEM, lalu HAPUS skip ini
      dan ganti dengan timeout Modbus lebih pendek.

      **Sisi app (SUDAH DIKERJAKAN):**
        - `home_screen.dart:132` pakai assignment langsung → ikut nilai dari Firebase,
          jadi otomatis benar begitu firmware kirim 0.
        - `anomaly_listener_service.dart` sudah baca flag `valid`:
          `final bool isValid = val['valid'] != false; if (!isValid) { _powerA = 0.0; ... }`

      ⚠️ **BELUM SELESAI — firmware belum kirim flag `valid`** (dicek 2026-09-25)
        App sudah siap menerima `valid: false` → reset ke 0. Tapi firmware
        (`src/main.cpp`) belum mengirim field itu sama sekali, dan skip PZEM
        (L341) masih ada. Jadi app belum bisa tahu data sudah basi.

        **Yang perlu dikerjakan di firmware:**
        1. Kirim `valid: true/false` di JSON `/energy/block_{a,b}`
           (`valid = false` saat PLN mati / `blockX.valid == false`)
        2. Atau pilih opsi A: tulis nol saat PLN mati
        Tanpa ini, bug "arus & daya stuck" masih terjadi walau app sudah siap.

di RTDB
- add tugangan zmpt ke firebase

di hardware
- buzzer PLN mati: ganti beep-beep (500ms/3s) jadi beep panjang 2 detik di awal mati lampu aja [x]
- PZEM VCC: 3.3V → 5V (spec PZEM-004T v3.0 = 5V, komunikasi gagal saat PLN mati)
- PZEM AC input: pastikan dari UPS (bukan PLN) kalau mau tetap monitor saat outage
- ZMPT potentiometer: kalibrasi ulang (putar sampai baseline ~2200 dan RMS 230+) [x] — hasil: PLN ON RMS 355.5 Vpp 937, PLN OFF RMS 6.5 (margin 53x)
- voltage_raw di Firebase: nilai RMS ADC mentah, belum dikonversi ke volt
- tulisan src di oled, geser ke kanan, jdi tulisan ups atau PLN itu mentok ke kanan [x]
- trus tulisan wifi: ON / OFF, ganti dengan tulisan ONLINE, OFFLINE. cek apakah cukup ruang di olednya? [x]
- ESP32 reboot (POWERON_RESET) saat PLN dicabut — cek apakah disupply dari UPS atau USB saat test

---

## Firmware
- [x] PZEM B baca data
- [x] SSR B pin dipindah ke GPIO27
- [x] Timestamp pakai NTP epoch seconds
- [x] Firmware hanya tulis `/energy/block_{a,b}`, app handle `/lamp/status/{a,b}`
- [x] Debounce Firebase command (2s) cegah flicker saat SSL reconnect
- [x] Boot: semua lamp OFF, baca status dari RTDB setelah connect
- [x] Lamp B bisa dikontrol dari app (sudah di code, perlu test)

## App (SmartNaga)
- [x] Align timestamp: firmware pakai epoch seconds → app parse `DateTime.fromMillisecondsSinceEpoch(timestamp * 1000)`
- [x] `getEnergyStream()` hanya baca `/energy/block_a` → tambah stream `/energy/block_b` (sudah listen langsung Blok A & B)
- [x] `home_bloc.dart`: tambah `energyB` + `lampBOn` ke state (arsitektur beralih ke stream langsung & session service)
- [x] `home_screen.dart`: tambah metrics grid Blok B + lamp toggle Blok B (total power & control screen per blok)
- [x] `control_screen.dart`: sudah OK (both blocks)
- [x] `historical_screen.dart`: tambah filter chip (1 Jam, 6 Jam, 1 Hari, 7 Hari, 30 Hari)
- [x] `settings_screen.dart`: align dengan design
