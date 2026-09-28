# Design: Decision Tree ke ESP32 (Firmware-First)

**Tanggal:** 2026-09-28
**Status:** DRAFT — menunggu approval

---

## 1. Tujuan

Pindahkan logika decision tree (10 status) dari Flutter app ke ESP32 firmware, sehingga OLED menampilkan status akurat 24/7 tanpa perlu app dibuka.

---

## 2. Arsitektur Otoritas

| Komponen | Tanggung Jawab |
|----------|----------------|
| **Firmware (ESP32)** | Evaluasi status real-time setiap 1 detik → OLED; publish ke RTDB `/lamp/diag/{a,b}` (optional) |
| **Flutter App** | Config (`expected_power`, `lamp_count`, `tolerance_percent`) → RTDB; notifikasi (FCM/local); notifikasi cooldown & recovery |
| **RTDB** | Config source of truth; telemetry; optional diagnostic mirror |

**Prinsip:** Firmware = single writer of OLED label; App = single writer of notifications + config. **Tidak ada RTDB field duplikat.**

---

## 3. Decision Tree Spec (Port Verbatim)

### Input
| Variabel | Sumber Firmware |
|----------|-----------------|
| `isRelayOn` | `lampAState` / `lampBState` |
| `voltage` | `blockX.voltage` (PZEM) — gated `usable` |
| `current` | `blockX.current` — gated `usable` |
| `actualPower` | `blockX.power` — gated `usable` |
| `expectedPower` | RTDB `/config/block_X/expected_power` (int, default 0) |
| `totalLamps` | RTDB `/config/block_X/lamp_count` (int, default 0) |
| `tolerancePercent` | RTDB `/config/block_X/tolerance_percent` (int, default 15) |
| `sensorOk` | `usable && plnPrevState` (bukan heartbeat!) |

### Tree (precedence urut)
```
1. !sensorOk → sensorError
2. !isRelayOn:
     actualPower>2.0 || current>0.05 → relayLeakage
     else → normalStandby
3. isRelayOn:
     voltage<50 → plnOutage
     voltage<190 → underVoltage
     voltage>245 → overVoltage
4. voltage normal (190-245):
     expectedPower<=0:
         actualPower<2.0 → circuitBroken
         else → normalActive
     expectedPower>0:
         powerPerLamp = expectedPower/max(totalLamps,1)  (fallback 9.0)
         lower = expected*(1-tol/100)
         upper = expected*(1+tol/100)
         actual < powerPerLamp*0.3 → circuitBroken
         actual in [lower, upper] → normalActive
         actual < lower → partialLampFail (estimatedDead clamp 1..totalLamps)
         else → overload
```

### Threshold (identik app)
| Konstanta | Nilai | Catatan |
|-----------|-------|---------|
| LEAK_POWER_W | 2.0 | relayLeakage threshold |
| LEAK_CURRENT_A | 0.05 | relayLeakage arus |
| PLN_OUTAGE_V | 50.0 | plnOutage |
| UNDER_VOLTAGE | 190.0 | underVoltage |
| OVER_VOLTAGE | 245.0 | overVoltage |
| NO_CONFIG_MIN_W | 2.0 | circuitBroken saat expected<=0 |
| DEFAULT_PPL_W | 9.0 | daya per lamp fallback |
| BROKEN_FRAC | 0.3 | circuitBroken saat config>0 |
| TOLERANCE_DEFAULT | 15.0 | % toleransi |
| DEAD_CLAMP_MIN | 1 | estimatedDead min |
| DEAD_CLAMP_MAX | totalLamps | estimatedDead max |

---

## 4. Firmware Changes (src/main.cpp)

### 4.1 Tambahan Struct & Enum
```cpp
enum class LampStatusFw : uint8_t {
  FW_SENSOR_ERROR, FW_PLN_OUTAGE, FW_UNDER_VOLTAGE, FW_OVER_VOLTAGE,
  FW_NORMAL_STANDBY, FW_NORMAL_ACTIVE,
  FW_CIRCUIT_BROKEN, FW_PARTIAL_LAMP_FAIL, FW_OVERLOAD,
  FW_RELAY_LEAKAGE
};

struct LampConfig {
  float expectedPower = 0.0f;
  int lampCount = 0;
  float tolerancePercent = 15.0f;
  bool loaded = false;
  unsigned long lastReadMs = 0;
};

struct EnergyData {
  // ... existing ...
  unsigned long lastGoodMs = 0;  // NEW: freshness gate
};
```

### 4.2 Helper Functions
- `lampStatusName(LampStatusFw)` → Dart enum name string (`"normalActive"` dll)
- `lampStatusShortLabel(LampStatusFw)` → OLED label ≤21 chars
- `evaluateLampStatusFw(relayOn, voltage, current, power, cfg, sensorOk)` — pure, port app tree
- `usable = plnPrevState && blockX.valid && fresh` (fresh = `millis()-lastGoodMs < 10000`)

### 4.3 Config Reader
```cpp
void readConfigOnce(const char* path, LampConfig& cfg) {
  FirebaseJson fbjson;
  if (Firebase.RTDB.getJSON(&fbdo, path, &fbjson)) {
    fbjson.get(cfg.expectedPower, "expected_power");
    fbjson.get(cfg.lampCount, "lamp_count");
    fbjson.get(cfg.tolerancePercent, "tolerance_percent");
    cfg.loaded = true;
    cfg.lastReadMs = millis();
  } else {
    // keep previous, mark stale
  }
}
```
Timer `timerConfig {0, 300000}` (5 menit) + one-shot `configBootRead` saat `firebaseReady` pertama kali.

### 4.4 Evaluator & OLED
- `updateDiagnostics()` di panggil di `updateOLED()` (1 detik)
- Power gating: `float effPower = usable ? blockX.power : 0.0f;`
- Settling 4 detik post-toggle: `unsigned long lampToggleMsA/B` di `setLampA/B()`
- Status short label: `PLN Padam`, `Standby`, `Bocor`, `Gangguan`, `Normal`, `Lampu Rusak`, `Beban Lebih`, `Teg. Rendah`, `Teg. Tinggi`, `Sensor Error`

---

## 5. RTDB Changes

### NO SCHEMA CHANGE
- **Tidak** tambah field ke `/energy/block_{a,b}` (setJSON erase)
- **Tidak** pakai `/lamp/status/{a,b}` (app-owned, rules-locked)
- **Optional (defer):** `/lamp/diag/{a,b}` — firmware writer, app reader. Butuh team consensus & rules update. **Defer ke phase 2.**

### Yang Tetap
- `/config/block_{a,b}` — app write, firmware read only
- `/energy/block_{a,b}`, `/energy/z_mpt`, `/energy/pln_status`, `/device/heartbeat`, `/device/ping` — unchanged
- `/lamp/control/{a,b}` — firmware reads, app writes command

---

## 5. Flash Budget

| Item | Estimasi |
|------|----------|
| Tree evaluator (if/else) | ~1.5 KB |
| Label table (10×15 chars) | ~150 B |
| Config reader + FirebaseJson calls | ~500 B |
| Freshness gate + settling | ~300 B |
| **Total tambahan** | **~2.5 KB** |
| Flash sisa (37 KB) | **Aman** |

**ArduinoJson: TIDAK dipakai** (0 symbols linked). Config parsing pakai `FirebaseJson` (sudah 90 symbols linked).

---

## 6. App Changes

| File | Perubahan |
|------|-----------|
| `decision_tree_evaluator.dart` | **Biarkan utuh** — fallback + unit test reference |
| `anomaly_listener_service.dart` | **Biarkan utuh** — evaluasi app tetap jalan; notifications & cooldown unchanged |
| `control_screen.dart` | **Biarkan** local fallback (722-748) — safety net untuk device belum reflash |
| `anomaly_config_screen.dart` | **Tidak ada** — masih writer `/config` |
| **Opsional (phase 2)** | Tambah listener `/lamp/diag` → set `diagnosticA/B` dari firmware |

---

## 6. Rollout Plan

1. **Firmware first** — upload, verifikasi OLED 5-label, verifikasi serial log tree output
2. **App follow-up** — (opsional) listener `/lamp/diag`, hapus evaluator & fallback lokal
3. **Rollback** — flash firmware lama; app tetap jalan (evaluator app masih ada)

---

## 7. Open Questions (Butuh Keputusan User)

1. **Threshold leakage: 5W (firmware now) vs 2W (app).** 3W load + relay OFF → OLED "Gangguan" vs App "Standby". Pilih:
   - [ ] Firmware → **2W** (selaras app)
   - [ ] App → **5W** (sekadar noise rejection, naikkan app)

2. **Under/Over Voltage di OLED (phase 1 vs phase 2).** Firmware punya `acVoltageReal` (ZMPT calibrated).
   - [ ] Masukkan sekarang (tambah 2 label: `Teg. Rendah`, `Teg. Tinggi`)
   - [ ] Defer ke phase 2 (tetap 5 label)

3. **Optional RTDB diagnostic path (`/lamp/diag`) — mau sekarang atau defer?**
   - [ ] Sekarang (butuh team consensus + rules update)
   - [ ] Defer (OLED sudah jalan, ini cuma untuk app cross-check)

---

## 8. Risks & Mitigasi

| Risk | Severity | Mitigasi |
|------|----------|----------|
| Flash overflow | 1 | FirebaseJson only; verify `pio run` flash after each edit |
| Threshold drift (5W vs 2W) | 1 | Resolve Q1 before merge; `docs/decision-tree-thresholds.md` |
| Stale power during PLN outage | 2 | `usable` gate (fresh + plnPrevState + valid) sudah di-design |
| Config read stall (10s timeout) | 2 | Gate on `firebaseReady`, set `timerConfig.prev` before call |
| Two evaluators diverging | 2 | Firmware authoritative when fresh; app fallback for null |
| OLED label overflow (11 chars) | 3 | Verify on hardware; fallback drop "Status: " prefix |

---

## 9. Out of Scope

- ArduinoJson dependency
- Notifications dari firmware (no FCM/APNs)
- `sensorError` di firmware (no "device offline" perspective)
- RTOS task / new timer beyond `timerConfig`
- Menghapus dead code (legacy screens, HomeBloc, FirebaseRealtimeDatasource)
- Fix `/system/reset_wifi` race, `home_screen.dart` energy=0.0 reset collision