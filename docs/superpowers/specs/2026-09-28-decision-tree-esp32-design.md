# Decision Tree on ESP32 — Hybrid Minimal-Duplication Design

**Date:** 2026-09-28
**Status:** Design (awaiting review)
**Repos:** firmware `kontrol lampu buah naga` (root), app `smartnaga_app`

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this design. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** The local OLED shows an accurate lamp status 24/7 without the phone app being open.

**Approach:** Hybrid / minimal-duplication. Keep the app's full 10-status tree for notifications and detail; give the firmware a 5-label authoritative core that covers what the OLED needs.

---

## 0. Reconciling the goal with the existing firmware

The firmware **already** has a 3-label local status (`lampStatusLabel()`, `src/main.cpp:503-507`) and **already** uses it on the OLED (`src/main.cpp:557, 562`). So this is not a green-field port — it is a **precision upgrade** of an existing local indicator, plus the config plumbing that lets it be accurate.

Current firmware label (`src/main.cpp:501-507`):
```cpp
static const float STATUS_POWER_MIN_W = 5.0;
const char* lampStatusLabel(bool commanded, float power) {
  bool hasLoad = power >= STATUS_POWER_MIN_W;
  if (commanded == hasLoad) return commanded ? "Normal" : "Standby";
  return "Gangguan";
}
```

The gap: with only relay-state and a 5W floor, the OLED says **"Normal" even when 3 of 5 lamps are dead**, as long as total power clears 5W. That is the concrete accuracy problem this design fixes.

---

## 1. The minimal authoritative core in firmware: 5 labels

The firmware evaluates **5 labels**. These are the ones the OLED can display in its 7-line, size-1 layout (`src/main.cpp:509-522`) and that a technician at the pole needs.

| # | Firmware label | App enum equivalent | Condition (in tree order) |
|---|---|---|---|
| 1 | `PLN Padam` | `plnOutage` | relay ON, `plnPrevState == false` |
| 2 | `Standby` | `normalStandby` | relay OFF, `power < LEAK_W` |
| 3 | `Bocor` | `relayLeakage` | relay OFF, `power >= LEAK_W` |
| 4 | `Gangguan` | `circuitBroken` | relay ON, PLN ok, `power < powerPerLamp * 0.3` (or `< 2.0W` when no config) |
| 5 | `Normal` / `Lampu Rusak` / `Beban Lebih` | `normalActive` / `partialLampFail` / `overload` | relay ON, PLN ok, power compared against `expectedPower ± tolerancePercent` |

Labels 5a/5b/5c are one branch in the code returning three different strings — the OLED needs a distinct string per outcome.

### 1.1 Thresholds — copied verbatim from the app

The firmware uses **exactly** the app's thresholds so the two agree by construction:

| Threshold | Value | App source |
|---|---|---|
| PLN outage | `plnPrevState == false` | firmware debounced ZMPT (`src/main.cpp:97, 132-133`) |
| Leakage / broken floor | `2.0 W` and `0.05 A` | `decision_tree_evaluator.dart:145` |
| No-config broken floor | `2.0 W` | `decision_tree_evaluator.dart:198` |
| powerPerLamp default | `9.0 W` | `decision_tree_evaluator.dart:214` |
| Broken fraction | `powerPerLamp * 0.3` | `decision_tree_evaluator.dart:219` |
| Tolerance band | `expectedPower * (1 ± tol/100)`, default `tol = 15` | `decision_tree_evaluator.dart:215-216, 120` |
| `estimatedDead` clamp | `[1, totalLamps]` | `decision_tree_evaluator.dart:243-244` |

### 1.2 ⚠️ Threshold conflict that already exists — must be decided

The firmware's current floor is **5.0 W** (`src/main.cpp:501`), but the app's leakage/broken floors are **2.0 W** (`decision_tree_evaluator.dart:145, 198`).

**This is a live, user-visible drift:** a 3 W load with the relay OFF shows **`Gangguan`** on the OLED today, but **`Lampu Standby`** in the app.

**Recommendation:** align firmware to the app's **2.0 W** (the app is the richer, authoritative tree; the firmware should be its coarsening, not a different rule set). Keep the `current > 0.05 A` leakage test too, since the firmware has `blockA.current` available (`src/main.cpp:85`) and the app uses it (`decision_tree_evaluator.dart:145`).

If the 5 W value was chosen for PZEM noise rejection, raise it in **both** trees in one change instead. Either way, the two must match — see §6.1.

### 1.3 What stays app-only (5 statuses)

| App-only status | Why it cannot move |
|---|---|
| `sensorError` | Needs heartbeat staleness + per-block energy-received flags + a 10s cold-start grace (`anomaly_listener_service.dart:358-369`). The ESP32's sensor is local and either reads or doesn't; there is no "device offline" concept for the device itself. |
| `underVoltage` (< 190V) | The firmware has `acVoltageReal` (ZMPT-calibrated, `src/main.cpp:137`) and could classify it — **deliberately deferred**, see §8. |
| `overVoltage` (> 245V) | Same. |
| Notification, `brokenLamps` count, 10-min cooldown, recovery notification | Needs persistence + OS notification stack; app-only by nature (`anomaly_listener_service.dart:415-451`). |
| The exact `farmerLabel` strings for the 10 statuses | The OLED uses 5 short strings; the app keeps its 10 verbose labels (`decision_tree_evaluator.dart:24-47`). |

**Note on `underVoltage`/`overVoltage`:** these are the two app statuses most worth showing on a 24/7 OLED (a brownout is exactly what a farmer would want to see at 3am). They are cheap in firmware — `acVoltageReal < 190` / `> 245` — but they change the label set from 5 to 7 and add OLED line pressure. Deferred to a follow-up phase so this change stays small; flagged here as the highest-value next increment.

---

## 2. How the two are prevented from disagreeing visibly

### 2.1 Single writer per concern

| Concern | Authoritative | Written to RTDB by |
|---|---|---|
| OLED label (5) | **Firmware** | nobody — computed and displayed locally only |
| App diagnostic (10) | **App** | nobody — in-memory `ValueNotifier` only (`anomaly_listener_service.dart:44-45`) |
| Relay state (SSR) | **Firmware** `lampAState`/`lampBState` | firmware (defaults at `src/main.cpp:311-312`) |
| Relay command | **App** | app → `/lamp/control/{a,b}` (`control_screen.dart:220`) |
| Config | **App** | app → `/config/block_{a,b}` (`anomaly_config_screen.dart:70-77`) |

Neither side publishes a derived status, so **there is no second source of truth to reconcile**. Agreement is achieved structurally: same inputs (RTDB energy + control + config), same thresholds (§1.1), firmware tree = coarsening of app tree.

### 2.2 The one case they can still differ, made explicit

Because the firmware sees only 5 buckets, `overload` and `underVoltage` both collapse into firmware's `Normal` or `Gangguan`. So:

> **OLED says `Normal`, app says `Beban Lebih` (overload) or `Tegangan Rendah`.**

This is not hidden, because:
- The app **already** renders its own diagnosis regardless of the OLED — `control_screen.dart:733-736` shows `diagnostic.status.farmerLabel` with `statusColor`, so an overload shows an orange "Beban Berlebih" badge in the app even while the OLED shows `Normal`.
- The app **never** reads or displays the firmware's OLED label, so it cannot be overridden by it.
- The OLED is a glance indicator; the app is the detail view. The coarsening is one-directional (firmware ⊂ app), never contradictory.

### 2.3 Why not publish the firmware status so the app can cross-check

Rejected for this phase: it adds an RTDB path, a rules change, and a second source of truth that *can* drift — all to detect a case (§2.2) that is already handled. See §3.3 for the deferred shape if the team later wants device-reported status.

---

## 3. RTDB paths and JSON shape

### 3.1 No new paths, no new writes

The firmware's 5-label status is **never written to RTDB**. It is computed inside `updateOLED()` and drawn. Therefore:

- **Zero schema change** → no team-consensus gate needed (AGENTS.md rule respected).
- **Zero collision risk** with the existing writers.

Existing firmware writes are unchanged: `/energy/block_a`, `/energy/block_b`, `/energy/z_mpt`, `/energy/pln_status`, `/device/heartbeat`, `/device/ping` (`src/main.cpp:626-663`).

### 3.2 The overwrite collision — explicitly avoided

`sendToFirebase()` uses `Firebase.RTDB.setJSON()` (`src/main.cpp:626, 636`), which **replaces the whole node** with a fresh `FirebaseJson` containing exactly `voltage, current, power, energy, valid, timestamp` (`src/main.cpp:619-625`).

**Consequence:** we must **not** add a `status`/`diagnostic` field under `/energy/block_{a,b}` — the next 2.5s write would erase it. This design adds no field there. (If the team ever wants firmware-derived data on RTDB, it goes on its own path, or `setJSON` must become a targeted `update()` — a separate change.)

### 3.3 Deferred option: `/device/oled_status/{a,b}` (NOT in this phase)

If the team later wants device-reported status in the app, the shape would be:

```json
"/device/oled_status/a": {
  "label": 2,          // int enum: 0=PLN Padam, 1=Standby, 2=Bocor, 3=Gangguan, 4=Normal, 5=Lampu Rusak, 6=Beban Lebih
  "power": 12.4,       // W, the value the label was derived from
  "config_loaded": true,
  "timestamp": 1758998400
}
```

Written by firmware in `sendToFirebase()`; requires a new `FIREBASE_RULES.md` block validating `hasChildren(['label','timestamp'])` with `label` an int (`FIREBASE_RULES.md:10-56` pattern). **Out of scope here** — listed so the decision is visible and can be taken deliberately later.

### 3.4 Config paths — unchanged, read-only from firmware

The app already writes flat keys (`anomaly_config_screen.dart:70-77`):

```
/config/block_a  →  { expected_power: int, lamp_count: int, tolerance_percent: int }
/config/block_b  →  { expected_power: int, lamp_count: int, tolerance_percent: int }
```

Firmware reads these **without modifying them**. No schema change.

---

## 4. How firmware gets config, and the fallback

### 4.1 Transport: `FirebaseJson`, not `ArduinoJson`

Recon (openQuestion in the firmware analysis) confirms `FirebaseJson` supports reading a JSON node into a target object, and `FirebaseJson` is **already linked** (1,465 symbols in the map), whereas `ArduinoJson` is declared but **not linked** and would pull ~30-50 KB flash + 2-4 KB RAM — which the 97.1% full flash (`platformio.ini:63`, `default.csv` app0 = 1.25 MB) cannot absorb.

**Decision: use `Firebase.RTDB.getJSON()` + `FirebaseJson` extraction. Do NOT add ArduinoJson.**

```cpp
// Exact API (mobizt Firebase 4.4.17, verified against the lib in .pio):
//   bool ok = Firebase.RTDB.getJSON(&fbdo, "/config/block_a", &energyJson);  // 3rd arg is target FirebaseJson
//   FirebaseJsonData d;
//   energyJson.get(d, "expected_power");
//   if (d.success) cfg.expectedPower = d.floatValue;            // member exists in json/FirebaseJson.h:447
//   energyJson.get(d, "lamp_count");   if (d.success) cfg.lampCount       = d.intValue;     // :448
//   energyJson.get(d, "tolerance_percent"); if (d.success) cfg.tolerancePercent = d.floatValue;
// `get()` returns false (and `success=false`, `error` set) when the key is absent —
// so a missing `tolerance_percent` leaves the 15.0 default in place.  // json/FirebaseJson.h:452-477
```

### 4.2 When it reads

| Trigger | When | Why |
|---|---|---|
| Boot (lazy one-shot) | first `loop()` tick where `firebaseReady == true && !configA.loaded` (or `configB`) | Config must be read after Firebase auth; lazy avoids a boot race with the `SYS_READY` state machine (`src/main.cpp:100-107`). Triggered via the `timerConfig` path below, not `setup()`. |
| Periodic refresh | every **10 minutes** via `timerConfig` (`600000 ms`) | Self-healing without new RTDB paths or app changes. Cost: one ~100-500 ms blocking read per 10 min, same order as the 6 sequential writes `sendToFirebase()` already does every 2.5s. |

**Read path detail.** A naive `Timer timerConfig{0, 600000}` would **not** fire at boot: `millis() - 0 >= 600000` is false until 10 minutes of uptime, so the OLED would show config-free labels for the first 10 minutes. Use an explicit boot trigger plus the 10-min refresh:

```cpp
// ── State (near src/main.cpp:97-98) ──
bool configBootRead = false;

// ── In loop(), after the timerOLED tick ──
// 1) One-shot read as soon as Firebase is ready (no 10-min wait at boot).
if (firebaseReady && !configBootRead) {
  configBootRead = true;
  timerConfig.prev = now;                       // start the 10-min clock from the boot read
  readConfigOnce("/config/block_a", configA);
  readConfigOnce("/config/block_b", configB);
}
// 2) Refresh every 10 minutes (self-heal / pick up app config edits).
if (firebaseReady && configBootRead && (now - timerConfig.prev >= timerConfig.interval)) {
  timerConfig.prev = now;                       // set BEFORE the read so a slow read can't re-trigger
  readConfigOnce("/config/block_a", configA);
  readConfigOnce("/config/block_b", configB);
}
```

`readConfigOnce()` sets `cfg.loaded = true` on any successful parse; a failed/missing read leaves the defaults from §4.3 in place and the next 10-min tick retries automatically.

A 10-minute cadence is chosen because the app **locks config editing while any relay is ON** (`anomalyParamLocked`, `anomaly_listener_service.dart:205-215`; UI lock at `anomaly_config_screen.dart:108-125,150`), so edits land while lamps are off and a ≤10-min firmware lag is harmless.

### 4.3 Missing / malformed config fallback

Matches the app exactly (`anomaly_listener_service.dart:380-382`, `decision_tree_evaluator.dart:196-211`):

| Situation | Firmware behaviour |
|---|---|
| Node absent, read timeout (10 s default), or parse failure | `cfg.loaded = false`; `expectedPower = 0.0`, `lampCount = 0`, `tolerancePercent = 15.0` |
| `expectedPower <= 0` | Use the app's no-config branch: `power < 2.0W` → `Gangguan`, else `Normal` (`decision_tree_evaluator.dart:196-211`) |
| `lampCount == 0` but `expectedPower > 0` | `powerPerLamp = 9.0` (`decision_tree_evaluator.dart:214`) |
| RTDB unreachable at boot | Keep defaults, retry on the next 10-min tick; OLED shows config-free labels meanwhile (never blocks, never crashes) |

The RTDB read must not hang the loop: `getJSON()` uses the library's 10 s default server-response timeout. To keep the OLED responsive, do the config read in the **same non-blocking slot pattern** as everything else (set the timer's `prev` *before* the call, so a slow read cannot re-trigger).

---

## 5. Firmware code structure and timers

### 5.1 Data structures (add near `src/main.cpp:83-98`)

```cpp
struct LampConfig {
  float expectedPower   = 0.0f;
  int   lampCount       = 0;
  float tolerancePercent = 15.0f;
  bool  loaded          = false;
};
LampConfig configA;
LampConfig configB;

enum LampStatusFw : uint8_t {
  FW_PLN_PADAM, FW_STANDBY, FW_BOCOR,
  FW_GANGGUAN, FW_NORMAL, FW_LAMPU_RUSAK, FW_BEBAN_LEBIH
};
```

### 5.2 Pure evaluation function (replaces `lampStatusLabel()`, `src/main.cpp:503-507`)

```cpp
// Cermin dari DecisionTreeEvaluator.evaluate() — SUBSET 5-label.
// Ambang WAJIB sama dengan decision_tree_evaluator.dart (lihat §1.1).
static const float LEAK_POWER_W = 2.0f;   // dulu 5.0 — disamakan dgn app
static const float LEAK_CURRENT_A = 0.05f;
static const float NO_CONFIG_MIN_W = 2.0f;
static const float DEFAULT_POWER_PER_LAMP_W = 9.0f;

LampStatusFw evaluateLampStatusFw(bool relayOn, float power, float current,
                                  bool plnAvailable, const LampConfig& cfg) {
  if (!relayOn)
    return (power > LEAK_POWER_W || current > LEAK_CURRENT_A) ? FW_BOCOR : FW_STANDBY;

  if (!plnAvailable) return FW_PLN_PADAM;

  if (cfg.expectedPower <= 0.0f)
    return (power < NO_CONFIG_MIN_W) ? FW_GANGGUAN : FW_NORMAL;

  const float perLamp = (cfg.lampCount > 0)
      ? cfg.expectedPower / cfg.lampCount : DEFAULT_POWER_PER_LAMP_W;
  const float lo = cfg.expectedPower * (1.0f - cfg.tolerancePercent / 100.0f);
  const float hi = cfg.expectedPower * (1.0f + cfg.tolerancePercent / 100.0f);

  if (power < perLamp * 0.3f) return FW_GANGGUAN;   // jalur putus / semua padam
  if (power >= lo && power <= hi) return FW_NORMAL; // dalam toleransi
  if (power < lo) return FW_LAMPU_RUSAK;            // sebagian lampu mati
  return FW_BEBAN_LEBIH;                            // di atas toleransi
}
```

`current` is passed for the leakage test (mirrors `decision_tree_evaluator.dart:145`).

### 5.3 Label mapping

```cpp
const char* statusLabelFw(LampStatusFw s) {
  switch (s) {
    case FW_PLN_PADAM:   return "PLN Padam";
    case FW_STANDBY:     return "Standby";
    case FW_BOCOR:       return "Bocor";
    case FW_GANGGUAN:    return "Gangguan";
    case FW_NORMAL:      return "Normal";
    case FW_LAMPU_RUSAK: return "Lampu Rusak";
    case FW_BEBAN_LEBIH: return "Beban Lebih";
  }
  return "?";
}
```

Longest label is `"Lampu Rusak"` (11 chars × 6 px = 66 px) — fits the 128 px OLED width after the `"Status: "` prefix (8 chars = 48 px) → 114 px. **`"Beban Lebih"` is also 11 chars → 114 px. OK.** If it overflows on hardware, drop the `"Status: "` prefix on those lines.

### 5.4 Timers — one new `Timer`, no new task

The firmware uses 8 non-blocking `millis()` timers (`src/main.cpp:66-80`). Add one:

```cpp
Timer timerConfig {0, 600000};  // Re-baca /config/block_{a,b} tiap 10 menit
```

Evaluation itself needs **no timer** — it runs inside `updateOLED()` (already 1000 ms, `src/main.cpp:378-381`), which is the only consumer. The tree is pure arithmetic, well under 1 ms.

### 5.5 Integration points (exact edits)

| # | Location | Change |
|---|---|---|
| 1 | `src/main.cpp:66-80` | add `timerConfig` |
| 2 | `src/main.cpp:83-98` | add `LampConfig` struct + `configA`/`configB` + enum |
| 3 | `src/main.cpp:147-162` | add forward decls: `readConfigOnce`, `evaluateLampStatusFw`, `statusLabelFw` |
| 4 | `src/main.cpp:503-507` | replace `lampStatusLabel()` body with `evaluateLampStatusFw()` + `statusLabelFw()` |
| 5 | `src/main.cpp:551-562` | gate `power` with `plnPrevState` as today, then call `statusLabelFw(evaluateLampStatusFw(lampAState, powerA, blockA.current, plnPrevState, configA))` |
| 6 | `loop()` near `src/main.cpp:383-387` | add `configBootRead` one-shot + `timerConfig` 10-min tick (§4.2) |
| 7 | `src/main.cpp:97-98` | add `bool configBootRead = false;` |

**Important:** keep the existing `plnPrevState && blockX.valid` power gate at `src/main.cpp:551-552` — `readPZEM()` is skipped when PLN is off (`src/main.cpp:351`), so `blockX.power` is stale then. Feeding stale power into the new tree would produce a false `Normal`/`Lampu Rusak` during an outage. The `FW_PLN_PADAM` branch is checked **before** any power comparison, which is the second layer of defence.

### 5.6 Estimated footprint

| Item | Flash |
|---|---|
| `evaluateLampStatusFw` + `statusLabelFw` | ~600 B |
| `LampConfig` + `readConfigOnce` (FirebaseJson extraction) | ~1.2 KB |
| `timerConfig` + integration | ~100 B |
| **Total** | **~2 KB** |

Against ~37 KB free (`1,310,720 - 1,273,233`). **Risk is not the tree — it is accidentally linking ArduinoJson.** Verify with `pio run` and check the map for `ArduinoJson` symbols remaining at 0.

---

## 6. App changes required

### 6.1 Required: none (behaviour-preserving)

The app's evaluator (`decision_tree_evaluator.dart`) and service (`anomaly_listener_service.dart`) are **unchanged**. All existing triggers, notifications, grace period, settling, and cooldown behaviour stays (`anomaly_listener_service.dart:122-142, 196-200, 278, 305-307, 391`).

### 6.2 Required if §1.2 is resolved toward 2.0 W: one firmware-only edit

If the team keeps the app's 2.0 W, only firmware changes. If the team prefers 5.0 W, **both** change — `decision_tree_evaluator.dart:145` and `:198`, plus `src/main.cpp:501`. Flagged as a decision, not silently picked.

### 6.3 Recommended: documentation comments (no behaviour change)

1. `control_screen.dart` near `_buildBlockStatusSection` (`:666`): note that the OLED shows a 5-label coarsening and the app may show a finer diagnosis; the app is the detail view.
2. New `docs/decision-tree-thresholds.md`: the single shared threshold table from §1.1, with a note that any change must be applied to **both** trees in one commit.

### 6.4 Explicitly not changed

- `/lamp/status/{a,b}` stays app-written `{state, timestamp}` (`control_screen.dart:194-199`); rules unchanged (`FIREBASE_RULES.md:23-36`).
- `/config/block_{a,b}` shape unchanged.
- No new app listener, no new RTDB read, no schema change.

### 6.5 Out of scope (separate work)

Dead code identified in recon — legacy `lib/presentation/screens/control/`, `home/`, `bloc/home/`, `data/datasources/firebase_realtime_datasource.dart` — is unrelated to this change and should be removed in its own PR.

---

## 7. Risks

| # | Risk | Severity | Mitigation |
|---|---|---|---|
| 7.1 | **Flash.** 97.1% full; a stray dependency blows the build. | High | Use `FirebaseJson` only; never `ArduinoJson`. Verify `pio run` + map after each edit. If over, drop the periodic config refresh first (§4.2) — the tree still works with defaults. |
| 7.2 | **Threshold drift.** Firmware and app classify the same reading differently (already true today: 5 W vs 2 W). | High | §1.2 decision + `docs/decision-tree-thresholds.md` + a comment cross-referencing both files. |
| 7.3 | **Config staleness.** Firmware lags app config by ≤10 min. | Low | Bounded by the 10-min tick; edits happen only while relays are OFF (`anomalyParamLocked`), so the window is idle. |
| 7.4 | **Stale power during PLN outage** feeding the tree. | Medium | Keep the `plnPrevState && blockX.valid` gate (`src/main.cpp:551-552`) **and** order `FW_PLN_PADAM` before power checks. |
| 7.5 | **Blocking RTDB read** (≤10 s worst case) stalling OLED/loop. | Medium | Set `timerConfig.prev` before the call; read at most every 10 min; the 10 s timeout is the same class as `sendToFirebase()`'s existing 6 writes. |
| 7.6 | **UX confusion** when OLED says `Normal` but app says `Beban Lebih`/`Tegangan Rendah`. | Low | §2.2 — app already shows its own badge (`control_screen.dart:733-736`) and never reads the OLED label; coarsening is one-directional. |
| 7.7 | **Schema consensus.** AGENTS.md requires consensus for RTDB schema changes. | None this phase | §3.1: no schema change. §3.3's option is deferred and gated on consensus. |
| 7.8 | **Long OLED strings** (`"Lampu Rusak"`, `"Beban Lebih"` = 11 chars). | Low | §5.3 width math; fallback is to drop the `"Status: "` prefix. Verify on hardware. |
| 7.9 | **ZMPT `voltage_real` unused** → brownouts invisible on OLED. | Medium (accepted) | §1.3 defers under/over-voltage; documented as the highest-value next increment. |

---

## 8. What this design deliberately does NOT do

- **Does NOT add ArduinoJson.** Uses the already-linked `FirebaseJson` (§4.1).
- **Does NOT write any diagnostic/status to RTDB.** No new path, no rules change, no consensus gate (§3.1).
- **Does NOT add fields under `/energy/block_{a,b}`.** `setJSON()` replaces the node every 2.5 s and would erase them (§3.2).
- **Does NOT replace the app's 10-status tree.** The app's evaluator and notifications are untouched (§6.1).
- **Does NOT add firmware notifications** or touch the buzzer logic (`setLampA/B`, `src/main.cpp:464-485`).
- **Does NOT port `sensorError`.** There is no meaningful "device offline" from the device's own perspective (§1.3).
- **Does NOT add `underVoltage`/`overVoltage` classification** this phase — deferred, despite being the highest-value next step (§1.3, §7.9).
- **Does NOT change `/lamp/status/{a,b}`** or its semantics as an app-written command echo (§6.4).
- **Does NOT add a new periodic task/RTOS task.** One extra `millis()` timer, evaluation folded into the existing 1 s OLED tick (§5.4).
- **Does NOT touch the app's dead code** (§6.5).
- **Does NOT resolve the 5 W vs 2 W conflict unilaterally** — it is surfaced as a decision (§1.2).

---

## 9. Implementation sequence

- [ ] **1. Decide §1.2** (5 W vs 2 W) with the team; record the choice in `docs/decision-tree-thresholds.md`.
- [ ] **2.** Add `LampConfig`, `configA`/`configB`, `LampStatusFw` enum (`src/main.cpp` ~line 83-98).
- [ ] **3.** Add `evaluateLampStatusFw()` + `statusLabelFw()`; replace `lampStatusLabel()` (`src/main.cpp:503-507`).
- [ ] **4.** Wire the new label into `updateOLED()` (`src/main.cpp:551-562`), preserving the `plnPrevState && valid` gate.
- [ ] **5.** Add `readConfigOnce()` using `Firebase.RTDB.getJSON()` + `FirebaseJson` extraction.
- [ ] **6.** Add `timerConfig` (600000 ms) and its `loop()` tick (`src/main.cpp` ~line 383-387).
- [ ] **7.** `pio run` (user runs it) → confirm flash ≤ 1,310,720 B and `ArduinoJson` symbols still 0 in the map.
- [ ] **8.** On hardware: verify all 5 labels on OLED, including a relay-OFF-with-load case and a PLN-off case.
- [ ] **9.** Add the app-side doc comment (§6.3.1) and `docs/decision-tree-thresholds.md` (§6.3.2).

- **Estimated firmware delta:** ~120 lines, < 2 KB flash. **App delta:** comments + docs only.
