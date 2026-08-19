# Buzzer Alarm Fix — PLN Mati

**Tanggal:** 2026-08-19
**Status:** Resolved

---

## Masalah

Saat PLN mati, buzzer harusnya beep periodik (500ms ON, 2s OFF, berulang). Yang terjadi:
- Buzzer nyala terus (tidak putus-putus)
- Beberapa kali buzzer Bunyi lebih lama dari seharusnya
- Timer OLED juga tidak update (jam skip 3-4 detik)

---

## Root Cause

**`readPZEM()` dan `sendToFirebase()` memblokir `loop()`.**

### Blocking Time Breakdown

| Sumber | Waktu | Keterangan |
|--------|-------|------------|
| `readPZEM()` | 100ms normal, **~7s saat PLN mati** | Modbus timeout — PZEM AC-powered, tidak ada jawaban |
| `sendToFirebase()` | **2-4 detik** | 6+ tulisan RTDB per cycle, REST blocking |
| `readZMPT()` | 30ms | `delayMicroseconds` × 400 sampel (cukup kecil) |
| **Total per loop** | **~10s** | Buzzer timer starved |

### Mekanisme Bug

```
loop() {
  checkFirebaseCommands();  // ← 100ms call, tapi bisa timeout
  sendToFirebase();         // ← 2-4 detik block
  updateOLED();             // ← 1 detik (untungnya ringan)
  buzzerCheck();            // ← tidak pernah jalan karena sebelumnya block 7-10 detik!
  readZMPT();               // ← 30ms, tapi alarm repeat ada di dalamnya
}
```

Ketika alarm trigger `buzzerOn(500)`:
1. `buzzerActive = true`, timer mulai
2. Loop masuk `sendToFirebase()` (blocking 2-4 detik)
3. Buzzer timer expiry **tidak diperiksa** selama blocking
4. Saat loop lanjut ke `buzzerCheck()`, timer sudah lewat — tapi buzzer tetap ON karena timer expiry diproses SETELAH blocking selesai
5. Alarm repeat di dalam `readZMPT()` juga terhalang blocking

**Analogi:** Timer buzzer kayak alarm jam, tapi orang yang harus cek alarm-nya sibuk 10 menit — begitu selesai, alarm sudah berulang 5x.

### Tambahan: PZEM Blocking

Saat PLN mati, PZEM tidak punya power (AC-powered). Modbus read timeout ~7 detik. `readPZEM()` dipanggil tiap 1 detik → setiap cycle block 7 detik → buzzer & OLED sama sekali tidak diproses.

### Tambahan: OLED Skip

`updateOLED()` hanya dijalankan sekali per loop iteration. Jika loop block 7 detik (PZEM timeout), OLED tidak update selama 7 detik → waktu lompat 3-4 detik di display.

---

## Solusi

### 1. Buzzer Check ke Paling Atas Loop

```cpp
void loop() {
  unsigned long now = millis();

  // Step 0: Buzzer — PALING PERTAMA, sebelum blocking call apapun
  if (buzzerActive && (now - timerBuzzer.prev >= buzzerDuration)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
  }

  // Step 0b: PLN alarm repeat
  if (plnAlarmActive && !buzzerActive && (now - lastPlnAlarm >= 2000)) {
    lastPlnAlarm = now;
    buzzerOn(500);
  }

  // Baru kemudian blocking calls:
  readPZEM();
  sendToFirebase();
  checkFirebaseCommands();
  updateOLED();
  readZMPT();
}
```

Buzzer timer diproses **sebelum** `readPZEM()` dan `sendToFirebase()` — buzzer OFF tepat waktu tanpa terganggu blocking.

### 2. Alarm Logic Split

Sebelumnya alarm repeat ada di dalam `readZMPT()` → terhalang oleh blocking di dalam loop. Sekarang:
- **Transition detection** (PLN→UPS): tetap di `readZMPT()` — hanya trigger sekali saat transisi
- **Alarm repeat** (tiap 2s): pindah ke `loop()` step 0 — selalu jalan tanpa terganggu

### 3. Skip `readPZEM()` Saat PLN Mati

```cpp
if (now - timerPZEM.prev >= timerPZEM.interval && acVoltage > ZMPT_THRESHOLD) {
  readPZEM();
}
```

PZEM AC-powered → tidak ada power → tidak ada data → skip. Menghemat ~7 detik blocking per cycle.

### 4. Alarm Guard: `!buzzerActive`

```cpp
if (plnAlarmActive && !buzzerActive && (now - lastPlnAlarm >= 2000)) {
```

Alarm hanya trigger `buzzerOn()` kalau buzzer sudah mati. Mencegah timer reset berulang → buzzer tidak pernah OFF.

### 5. Hapus Buzzer Queue

Sebelumnya ada queue (`buzzerPending`) untuk handle A+B toggle berbarengan. Tapi queue menyebabkan buzzer nyala lebih lama dari seharusnya. Dihapus — `buzzerOn()` langsung override timer.

### 6. Default Lampu OFF saat Boot

```cpp
lampAState = false;
lampBState = false;
controlSSR();
controlSSRB();
```

Sebelum WiFi connect, lampu OFF — tidak tergantung Firebase state.

---

## Wiring Update

| Komponen | Pin Lama | Pin Baru | Alasan |
|----------|----------|----------|--------|
| SSR_B | GPIO27 | GPIO4 | Avoid buzzer noise (GPIO23) |
| Buzzer | GPIO23 | GPIO23 | Tetap (sudah jauh dari SSR_B baru) |

---

## Serial Monitor Debug Format

```
[detik][KOMPONEN] data
[30][ALARM] PLN MATI!
[30][BUZZ] 500ms
[35][BUZZ] off after 500ms
```

---

## Ringkasan Perubahan

| Perubahan | Sebelum | Sesudah |
|-----------|---------|---------|
| Buzzer check position | Step 6 (setelah blocking) | Step 0 (pertama) |
| Alarm repeat location | Di dalam `readZMPT()` | Di `loop()` step 0 |
| PZEM read saat PLN mati | Selalu (block 7s) | Skip |
| Buzzer On() | Queue + override | Override langsung |
| Alarm beep duration | 200ms | 500ms (distinct dari toggle) |
| SSR_B pin | GPIO27 | GPIO4 |
| Default lampu boot | Tidak ditentukan | OFF |

---

## Kode Original vs Fixed

### Buzzer Timer Check (Original)
```cpp
// Step 6 — setelah sendToFirebase blocking 2-4 detik
if (buzzerActive && (now - timerBuzzer.prev >= buzzerDuration)) {
  digitalWrite(BUZZER_PIN, LOW);
  buzzerActive = false;
}
```

### Buzzer Timer Check (Fixed)
```cpp
// Step 0 — paling pertama di loop()
if (buzzerActive && (now - timerBuzzer.prev >= buzzerDuration)) {
  digitalWrite(BUZZER_PIN, LOW);
  buzzerActive = false;
}
```

---

## Test Cases

| Test | Expected | Result |
|------|----------|--------|
| Toggle A ON/OFF | Buzzer beep 200ms | ✅ |
| Toggle B ON/OFF | Buzzer beep 200ms | ✅ |
| Toggle A+B bersamaan | Kedua beep 200ms | ✅ |
| PLN mati → UPS | Beep 500ms, OFF 2s, repeat | ✅ |
| PLN kembali | Alarm berhenti | ✅ |
| Boot tanpa WiFi | Lampu OFF, buzzer beep | ✅ |
| OLED time | Update tiap 1 detik | ✅ |
