# ZMPT Firmware Improvements Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix ZMPT voltage sensing instability, boot alarm false trigger, and PZEM reading issues to achieve reliable PLN/UPS detection and stable RMS measurements.

**Architecture:** 
- Consolidate ZMPT baseline+RMS into single sampling window to eliminate transient errors during PLN transitions
- Increase ADC sample count from 200 to 500 for better noise rejection and stable RMS
- Initialize `plnPrevState` from actual first ZMPT reading to prevent boot alarm false positives
- Make PZEM-B reads unconditional (since it's UPS-powered) while keeping PZEM-A skip logic
- Add proper ADC configuration (`analogReadResolution`, `analogSetAttenuation`) missing in main firmware
- Instrument reset-reason logging to diagnose POWERON_RESET events on PLN cut

**Tech Stack:** ESP32 Arduino core, PZEM-004T v3.0 library, ADC1_CH6 (ZMPT), HardwareSerial (PZEM)

**Spec:** Derived from todo.md hardware notes, TROUBLESHOOTING.md, and main.cpp/firmware analysis

## Global Constraints
- Maintain existing non-blocking timer architecture (millis() based)
- Preserve PZEM update interval (2.5 seconds) and ZMPT read interval (100ms)
- Keep OLED/Firebase/Firebase update cadence unchanged
- Do not break existing SSR/lamp control logic
- ESP32 must remain powered from UPS 5V step-down during PLN outages

## Review Focus
- **Transient PLN/UPS misdetection:** Verify single-window ZMPT eliminates false state flips during voltage sags
- **Boot alarm silence:** Confirm no 2-second beep when device boots with PLN disconnected
- **PZEM-B data continuity:** Ensure Blok B energy readings continue during PLN outages
- **ADC configuration:** Confirm 12-bit resolution and 0-3.1V range via attenuation setting
- **Reset diagnostics:** Validate POWERON_RESET cause is logged when PLN is cut

---
### Task 1: Add missing ADC configuration and reset-reason logging

**Files:**
- Modify: `src/main.cpp:129-130` (add ADC config after Serial.begin)
- Modify: `src/main.cpp:185-186` (add reset reason print after setup banner)
- Modify: `src/main.cpp:608-611` (ensure printResetReason called during brownout investigation)

**Interfaces:**
- Consumes: `esp_reset_reason()` from ESP32 Arduino core
- Produces: Reset reason logged to serial at boot; ADC configured for 12-bit 0-3.1V range

- [ ] **Step 1: Add ADC config in setup()**

```cpp
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== SmartNaga ESP32 Boot ===");

  // ADC configuration for ZMPT101B (GPIO34)
  analogReadResolution(12);           // 12-bit resolution (0-4095)
  analogSetAttenuation(ADC_11db);     // Input voltage range ~0 - 3.1V
```

- [ ] **Step 2: Add reset-reason logging after boot banner**

```cpp
  Serial.println("\n=== SmartNaga ESP32 Boot ===");
  printResetReason();  // Log reset cause for debugging POWERON_RESET
```

- [ ] **Step 3: Verify printResetReason function exists**

```cpp
void printResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset Reason: %d\n", reason);
  // Optional: map enum to string for readability
}
```

- [ ] **Step 4: Commit**

```bash
git add src/main.cpp
git commit -m "feat: add ADC config and reset-reason logging for ZMPT stability"
```

### Task 2: Fix ZMPT baseline+RMS to single-window algorithm

**Files:**
- Modify: `src/main.cpp:646-695` (replace readZMPT() function body)

**Interfaces:**
- Consumes: 500-600 ADC samples in single pass
- Produces: stable baseline, RMS, peak-to-peak, and plnAvailable boolean
- Called by: timerZMPT every 100ms

- [ ] **Step 1: Replace two-pass sampling with single-pass Welford's algorithm**

```cpp
void readZMPT() {
  // Safety net: cek buzzer-OFF sebelum blocking ADC sampling
  unsigned long ms = millis();
  if (buzzerActive && (ms - timerBuzzer.prev >= buzzerDuration)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    buzzerOffTime = ms;
  }

  // Single-pass sampling: compute baseline (mean) and RMS in one loop
  const uint16_t SAMPLE_COUNT = 500; // Increased from 200 for better noise rejection
  float sum = 0.0f;
  float sumSq = 0.0f;
  int minVal = 4095, maxVal = 0;

  for (uint16_t i = 0; i < SAMPLE_COUNT; i++) {
    int raw = analogRead(ZMPT_PIN);
    sum += raw;
    sumSq += (float)raw * raw;
    if (raw < minVal) minVal = raw;
    if (raw > maxVal) maxVal = raw;
    delayMicroseconds(100); // Maintain ~100us delay per sample as before
  }

  float baseline = sum / SAMPLE_COUNT;
  float variance = (sumSq / SAMPLE_COUNT) - (baseline * baseline);
  float rms = sqrt(fmax(variance, 0.0f)); // Protect against floating-point negatives
  int peakToPeak = maxVal - minVal;

  // Invalid signal detection (same thresholds as before)
  bool plnAvailable = false;
  if (rms >= 30.0 && peakToPeak >= 20 && rms <= 500.0 && rms > ZMPT_THRESHOLD) {
    plnAvailable = true;
  }

  // acVoltage for display: show RMS when signal present, else 0
  acVoltage = (plnAvailable || rms >= 30.0) ? rms : 0.0;

  // Throttled serial print (every 5 seconds)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 5000) {
    lastPrint = millis();
    const char* src = plnAvailable ? "PLN" : "UPS";
    Serial.printf("[%lu][ZMPT] RMS:%.1f P2P:%d Base:%.1f Src:%s\n", 
                  millis()/1000, rms, peakToPeak, baseline, src);
  }

  // PLN loss/gain alarm logic (edge-triggered, 2s beep on loss)
  if (plnPrevState && !plnAvailable) {
    buzzerOn(2000);
    Serial.printf("[%lu][ALARM] PLN MATI!\n", millis()/1000);
  } else if (!plnPrevState && plnAvailable) {
    Serial.printf("[%lu][ALARM] PLN KEMBALI\n", millis()/1000);
  }
  plnPrevState = plnAvailable;
}
```

- [ ] **Step 2: Verify single-pass math matches original dual-pass intent**

- [ ] **Step 3: Commit**

```bash
git add src/main.cpp
git commit -m "feat: replace ZMPT dual-pass baseline/RMS with single-pass 500-sample algorithm"
```

### Task 3: Fix boot alarm by initializing plnPrevState from first reading

**Files:**
- Modify: `src/main.cpp:78` (change plnPrevState initialization)
- Modify: `src/main.cpp:180-182` (add early ZMPT read before lamp control init)

**Interfaces:**
- Consumes: First ZMPT reading after ADC config
- Produces: Correct initial plnPrevState reflecting actual power source at boot

- [ ] **Step 1: Change plnPrevState initialization to read actual state**

```cpp
// ── State ────────────────────────────────────────────────────────────────────
struct EnergyData {
  // ... existing fields ...
};

EnergyData blockA;
EnergyData blockB;
bool lampAState = false;
bool lampBState = false;
bool wifiConnected = false;
bool firebaseReady = false;
// Initialize plnPrevState by reading actual ZMPT state (will be set properly in setup)
bool plnPrevState = false; // Temporary, will be overwritten
bool inConfigPortal = false;
```

- [ ] **Step 2: Add early ZMPT read in setup() after ADC config but before SSR init**

```cpp
  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // I2C OLED — init awal, tampilkan boot screen
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] SSD1306 allocation failed");
  } else {
    // ... OLED init ...
  }

  // EARLY ZMPT READ: establish initial plnPrevState before any alarms
  readZMPT(); // This sets plnPrevState based on actual first measurement

  // RTC init
  // ... RTC code ...
```

- [ ] **Step 3: Remove duplicate ZMPT read that was causing reset (if any)**

- [ ] **Step 4: Commit**

```bash
git add src/main.cpp
git commit -m "fix: initialize plnPrevState from actual ZMPT reading to prevent boot alarm false trigger"
```

### Task 4: Make PZEM-B reads unconditional (UPS-powered)

**Files:**
- Modify: `src/main.cpp:315-319` (PZEM-A conditional read logic)
- Modify: `src/main.cpp:366-413` (readPZEM() function - remove skip condition for PZEM-B)

**Interfaces:**
- Consumes: acVoltage > ZMPT_THRESHOLD for PZEM-A skip logic
- Produces: PZEM-B reads always occur; PZEM-A reads skip when PLN off (AC-powered)
- Called by: timerPZEM every 2.5 seconds

- [ ] **Step 1: Keep PZEM-A skip logic but add explicit comment**

```cpp
// 1. Baca PZEM tiap 2,5 detik — skip saat PLN mati (PZEM AC-powered, read akan block ~7s)
// PZEM-B di UPS power → selalu baca
if (now - timerPZEM.prev >= timerPZEM.interval) {
  timerPZEM.prev = now;
  readPZEM(); // Now handles internal skipping for PZEM-A only
}
```

- [ ] **Step 2: Modify readPZEM() to skip only PZEM-A when PLN off**

```cpp
void readPZEM() {
  // ... address printing debug (unchanged) ...

  // Baca data Blok B (always read - UPS powered)
  blockB.voltage = pzemB.voltage();
  blockB.current = pzemB.current();
  blockB.power   = pzemB.power();
  blockB.energy  = pzemB.energy();
  blockB.valid = !isnan(blockB.voltage) && !isnan(blockB.current);

  if (blockB.valid) {
    printEnergyData(blockB, "Blok B");
  } else {
    Serial.println("[PZEM-B] No valid data");
  }

  // Baca data Blok A (skip saat PLN mati karena AC-powered)
  if (acVoltage > ZMPT_THRESHOLD) { // PLN ON
    blockA.voltage = pzemA.voltage();
    blockA.current = pzemA.current();
    blockA.power   = pzemA.power();
    blockA.energy  = pzemA.energy();
    blockA.valid = !isnan(blockA.voltage) && !isnan(blockA.current);

    if (blockA.valid) {
      printEnergyData(blockA, "Blok A");
    } else {
      Serial.println("[PZEM] Error: Gagal baca data Blok A");
    }
  } else {
    // PLN OFF: mark Blok A invalid but don't spam errors
    blockA.valid = false;
    // Optional: infrequent debug to confirm skip is working
    static unsigned long lastSkipLog = 0;
    if (millis() - lastSkipLog >= 30000) { // Every 30 seconds
      lastSkipLog = millis();
      Serial.println("[PZEM-A] Skipped (PLN off)");
    }
  }
}
```

- [ ] **Step 3: Verify PZEM-B continues delivering data during PLN outages**

- [ ] **Step 4: Commit**

```bash
git add src/main.cpp
git commit -m "feat: make PZEM-B reads unconditional (UPS-powered) while preserving PZEM-A skip logic"
```

### Task 5: Add calibration factor and threshold validation notes (post-fix)

**Files:**
- Create: `docs/superpowers/plans/2026-09-25-zmpt-followup.md` (for future calibration work)

**Interfaces:**
- No code changes in this task — documentation for post-implementation verification

- [ ] **Step 1: Document post-fix validation steps**

```markdown
# ZMPT Calibration Validation (Post-Fix)

After implementing the core fixes, perform these validation steps:

1. **Threshold re-check:** With stable 500-sample RMS, verify ZMPT_THRESHOLD=150 still provides adequate margin
   - Measure PLN ON RMS (should be ~350-400 raw ADC)
   - Measure PLN OFF RMS (should be <10 raw ADC)
   - Adjust threshold if needed (recommended: midpoint with safety margin)

2. **Calibration factor k:** Once RMS is stable, compare to known good multimeter
   - k = Vrms_measured / Vrms_multimeter
   - Expect k ~ 0.65 based on voltage divider (validate resistor values)
   - If stable, apply: Vrms_true = k * Vrms_raw

3. **Jitter test:** Monitor RMS standard deviation over 10-second window
   - Target: <5% variation at steady state
   - If still high, consider increasing sample count to 800-1000

4. **PLN transition test:** Simulate voltage sag and verify no false tripping
   - Use variac or programmable load to create dips
   - Confirm hysteresis prevents chattering near threshold
```

- [ ] **Step 2: Commit documentation**

```bash
git add docs/superpowers/plans/2026-09-25-zmpt-followup.md
git commit -m "docs: add ZMPT calibration validation steps for post-fix verification"
```

### Task 6: Verify and test all changes

**Files:**
- Test: Compile and upload to ESP32
- Test: Monitor serial output for reset reason, ADC config, ZMPT readings
- Test: Boot with PLN on/off to verify no false alarm
- Test: Cut PLN power and verify PZEM-B continues, PZEM-A skips
- Test: Log reset reason when PLN is cut to diagnose POWERON_RESET

**Interfaces:**
- Consumes: All modified firmware
- Produces: Working firmware with fixes applied

- [ ] **Step 1: Build firmware**

```bash
pio run -e esp32dev
```

- [ ] **Step 2: Upload and monitor initial boot**

```bash
pio run -t upload -e esp32dev && pio monitor -e esp32dev
```

- [ ] **Step 3: Verify boot output shows:**
  - ADC configuration applied (no error messages)
  - Reset reason logged (should show 0=POWERON or 12=SW during normal boot)
  - No 2-second buzzer beep on boot when PLN is intentionally off
  - Stable ZMPT RMS readings (jitter reduced from ±20 to ±5 raw counts)
  - PZEM-B data continuing during simulated PLN outage (disconnect AC input to PZEM-A/UPS but keep UPS power to ESP32/PZEM-B)

- [ ] **Step 4: Commit test verification**

```bash
git add -u
git commit -m "test: verify ZMPT firmware improvements function correctly"
```

---
**Plan complete and saved to `docs/superpowers/plans/2026-09-25-zmpt-firmware-improvements.md`. Please review the plan. Which execution approach would you prefer?**

- **Subagent-driven** - A fresh subagent implements each task and a fresh reviewer checks it before the next one starts, then a whole-branch review at the end. Most thorough; costs a fresh context per task and per review.
- **Native** - I implement every task myself in this session, the way this harness runs work, then one fresh reviewer on the most capable model checks the whole branch. Cheapest and fastest; no independent review until the end. Runs well with a mid-tier session model, since the plan carries the design.

**For this plan I recommend Native, because the tasks are tightly coupled (single file modifications) and mostly involve algorithmic replacements in one function. Does the plan capture what you want, and which approach should we use?**