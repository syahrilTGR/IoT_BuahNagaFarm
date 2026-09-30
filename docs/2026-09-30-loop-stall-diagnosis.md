# Diagnosa Loop Stall ESP32 — 2026-09-30

**Status:** Sementara (mitigasi terpasang, akar masalah belum hilang total)
**Sesi:** 2026-09-30, 12:55–14:35 WIB

---

## 1. Gejala (dilaporkan user)

- Detik di OLED **berhenti** beberapa detik, lalu kembali berjalan
- Status device di app **offline** selama itu, lalu **online** lagi
- Terjadi **berkala**, bukan sekali

Dugaan awal user: "hardware berhenti". Perlu dibuktikan apakah stall atau reset.

---

## 2. Apa yang dilakukan (kronologis)

### 2.1 Instrumentasi awal (sebelum fix)

| Langkah | Tujuan |
|---|---|
| Panggil `printResetReason()` di `setup()` | Membedakan **reset** vs **stall** — sebelumnya tidak pernah dipanggil |
| Tambah macro `fbTimed()` untuk bungkus 13 call Firebase | Ukur durasi tiap operasi RTDB, cetak bila > 500ms |
| Serial monitor eksternal (pyserial) + stall detector | Deteksi gap timing > 2.5× interval normal |

**Sinyal yang dipantau** (masing-masing punya interval tetap):

| Sinyal | Interval | Dijalankan dari |
|---|---|---|
| `[PING]` | 3s | task esp_ping (FreeRTOS terpisah) |
| `[Blok A/B]` | 2.5s | `loop()` → `readPZEM()` |
| `[DIAG]` | 5s | `loop()` → `updateOLED()` → `updateDiagnostics()` |
| `[ZMPT]` | 5s | `loop()` → `readZMPT()` |

**Kunci pembeda:** kalau sinyal `loop()` berhenti tapi `[PING]` (task terpisah) masih jalan → loop yang stall, RTOS hidup.

### 2.2 Fix yang dicoba

**Fix 1 — `Firebase.RTDB.setReadTimeout(&fbdo, 5000)`**

- Hipotesis: default `read_tmo = -1` → fallback 30s, terlalu panjang
- **Hasil: GAGAL.** Durasi stall tetap 61–66s, tidak berubah sama sekali.
- **Sebab:** `setReadTimeout` mengatur `read_tmo` (fase *baca response server*). Yang menggantung ada di fase **SSL write** (`BR_SSL_SENDAPP`), yang diatur mekanisme berbeda.

**Fix 2 (Opsi A) — `timerCheckCmd` 100ms → 1000ms**

- Hipotesis: `checkFirebaseCommands()` melakukan 2 HTTPS GET tiap 100ms = **20 request/detik** lewat 4G. Tiap request adalah peluang menggantung di SSL write.
- **Hasil: BERHASIL SEBAGIAN.** Durasi stall 66s → **13s**, SSL timeout 6 → **0**, frekuensi turun.

---

## 3. Hasil

### 3.1 Bukti akar masalah (dari instrumentasi)

```
[FB-SLOW] getBool /lamp/control/a: 60043ms (ok=0)
```

`checkFirebaseCommands()` → `getBool /lamp/control/a` menggantung **60,043 detik** — persis durasi stall. Error SSL muncul tepat di tengahnya:

```
> WARN.mRunUntil: Terminating because the ssl engine closed.
> ERROR.write: Failed while waiting for the engine to enter BR_SSL_SENDAPP.
> ERROR.mRunUntil: SSL internals timed out!
> ERROR.mConnectSSL: Failed to initlalize the SSL layer.
```

### 3.2 Mekanisme (dibaca dari source library)

- `client/SSLClient/client/BSSL_SSL_Client.h:371` → `_handshake_timeout = 60000` (default **60s**)
- `BSSL_SSL_Client.cpp:1593` → `mRunUntil(BR_SSL_SENDAPP, _handshake_timeout)`
- Artinya: bila peer tidak siap menerima (jaringan 4G bermasalah), BearSSL menunggu sampai 60s
- Library **tidak pernah** memanggil `setHandshakeTimeout()` → nilai tetap 60s
- `config.timeout.sslHandshake` di `FB_Const.h:1307` bertanda **"unused"** → tidak mengikat

### 3.3 Perbandingan terukur

> **Catatan hitungan:** detektor mencatat STALL **per sinyal** — satu episode stall
> yang menghentikan loop akan memicu 3–4 baris STALL (ping, readPZEM, readZMPT,
> updateDiagnostics). Angka "episode" di bawah dihitung dari gap millis `[ZMPT]`,
> jadi satu episode = satu angka.

| Metrik | Sebelum (poll 100ms) | Sesudah (poll 1s) |
|---|---|---|
| Episode stall (dari gap ZMPT) | 3 episode / 16 mnt | **1 episode / 35 mnt** |
| Durasi stall maks | **66s** | **13s** |
| SSL timeout | 6 | **0** |
| `getBool /lamp/control/a` maks | 60043ms | 10012ms |
| Crash / reset | 0 | 0 |

Setelah 14:19 tidak ada episode stall lagi selama **21 menit** berikutnya.

### 3.4 Yang terbukti (bukan dugaan)

1. **Bukan reset** — tidak ada boot banner di seluruh log; `millis()` terus naik
2. **Bukan WDT** — chip tidak pernah restart
3. **Loop yang stall**, RTOS tetap hidup — `[PING]` (task terpisah) pulih sendiri
4. **Pemicu pasti:** operasi Firebase di `loop()` menggantung di fase SSL write
5. **`setReadTimeout` tidak menyentuh fase itu** — terbukti tidak berpengaruh

---

## 4. Perubahan kode saat ini (belum di-commit)

| File | Perubahan |
|---|---|
| `src/main.cpp` | `printResetReason()` di `setup()` + forward decl |
| `src/main.cpp` | `Firebase.RTDB.setReadTimeout(&fbdo, 5000)` — **tidak efektif, kandidat revert** |
| `src/main.cpp` | `timerCheckCmd` 100 → 1000ms |
| `src/main.cpp` | Instrumen `fbTimed()` membungkus 13 call Firebase (temporary) |
| `platformio.ini` | Partisi `default.csv` → `huge_app.csv` (app0 1.25MB → 3MB) |

---

## 5. Yang belum selesai / perlu keputusan

### 5.1 Akar masalah belum hilang

Opsi A hanya **mengurangi peluang** (request 10× lebih jarang). Mekanisme SSL write hang masih ada — stall 13s masih bisa terjadi.

### 5.2 Opsi lanjutan

**Opsi B — atur timeout yang benar**
- `setHandshakeTimeout()` ada di `BSSL_SSL_Client`, tapi library tidak mengeksposnya lewat `config`
- Perlu cek apakah bisa dijangkau dari kode aplikasi
- Risiko: perlu patch/reach-into library

**Opsi C — pindahkan Firebase ke task FreeRTOS terpisah** (solusi paling benar)
- `loop()` (OLED, ZMPT, PZEM, relay) tidak pernah stall
- Task Firebase boleh stall sendiri tanpa mengganggu UI
- Perubahan ~100 baris, risiko menengah
- Konsekuensi: heartbeat tetap tertunda saat stall (tapi OLED tidak beku)

### 5.3 Instrumen perlu dibersihkan

`fbTimed()` membungkus 13 call — cukup berguna untuk diagnosa, tapi menambah overhead. Pilihan: hapus, atau pertahankan (hanya print saat > 500ms, relatif ringan).

### 5.4 Uji lapangan yang belum dilakukan

- Tes boot-sync: set lampu ON dari app → restart ESP → cek relay menyala
- Tes PLN mati → pastikan arus/daya reset ke 0 di app
- Pantau jangka panjang (1–2 minggu) untuk lihat apakah stall 13s masih mengganggu

---

## 6. Alat diagnostik (bisa dipakai ulang)

| File | Fungsi |
|---|---|
| `/tmp/esp32_stall_monitor.py` | Baca serial + tulis log + deteksi stall (threshold terkalibrasi per sinyal) |
| `/tmp/esp32_monitor.log` | Log serial mentah |
| `/tmp/esp32_events.log` | Hanya event penting (STALL/CRASH/SIGNAL) |

Catatan: `pio device monitor` **tidak bisa** dipakai non-interaktif (butuh TTY). Pakai pyserial langsung.

---

## 7. Pelajaran

1. **`setReadTimeout` bukan obat untuk stall SSL write.** Timeout berbeda mengatur fase berbeda — baca source, jangan tebak.
2. **Jendela tenang singkat bukan bukti.** Sempat 2× menyimpulkan "0 stall" padahal stall muncul beberapa menit kemudian. Butuh ≥30 menit untuk klaim.
3. **Instrumentasi per-call mengubah dugaan jadi bukti.** Angka `60043ms` langsung menunjuk call pelakunya.
4. **`printResetReason()` harus ada sejak awal** — tanpa itu, stall dan reset tampak sama.
