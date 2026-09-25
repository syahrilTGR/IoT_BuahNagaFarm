// ════════════════════════════════════════════════════════════════════════════
// ZMPT101B Hardware Diagnostic Tool
// ════════════════════════════════════════════════════════════════════════════
// Tujuan: kalibrasi potensiometer ZMPT101B (gain) sebelum dipakai di firmware
//         utama. Membaca Min/Max/Midpoint/Vpp + RMS selama 1 detik.
//
// Cara pakai:
//   pio run -t upload -e test_zmpt
//   pio monitor -e test_zmpt
//
// Target kalibrasi (untuk PLN 220-240V):
//   - Vpp  : 600 - 1200  (ideal ~800)
//   - Mid  : ~2048 (tengah ADC 12-bit)
//   - RMS  : > 150  (threshold firmware utama = 150)
//
// ── HASIL KALIBRASI (2026-09-25) ────────────────────────────────────────────
//   Kondisi   | RMS    | Vpp | Base | Mid
//   ----------|--------|-----|------|------
//   PLN ON    | 355.5  | 937 | 1795 | 1763
//   PLN OFF   |   6.5  |  48 | 1805 | 1801
//
//   Margin: 53x (355.5 / 6.5) — threshold 150 di tengah, aman.
//   Verifikasi: RMS = Vpp/2√2 = 937/2.828 = 331 vs terukur 355 (+7%,
//   wajar karena sampling tidak sinkron sempurna dengan 50 Hz).
// ════════════════════════════════════════════════════════════════════════════

#include <Arduino.h>

#define ZMPT_PIN 34

// ── Ambang diagnostik ────────────────────────────────────────────────────────
static const int ADC_MAX = 4095;
static const int ADC_MIN = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);  // Rentang pembacaan ~0 - 3.1V

  Serial.println();
  Serial.println("==============================================");
  Serial.println("   DIAGNOSTIK HARDWARE ZMPT101B");
  Serial.println("==============================================");
  Serial.println("Pin ADC     : GPIO34 (ADC1_CH6)");
  Serial.println("Resolusi    : 12-bit (0-4095)");
  Serial.println("Attenuation : 11db (~0-3.1V)");
  Serial.println("Target Vpp  : 600-1200 (ideal ~800)");
  Serial.println("Target Mid  : ~2048");
  Serial.println("Target RMS  : > 150");
  Serial.println("----------------------------------------------");
}

void loop() {
  int v_min = ADC_MAX;
  int v_max = ADC_MIN;

  long sum_raw = 0;      // untuk baseline
  long sum_sqr = 0;      // untuk RMS
  long n_samples = 0;

  // Ambil sampel selama 1000 ms (50 siklus jala-jala 50 Hz)
  uint32_t start_time = millis();
  while (millis() - start_time < 1000) {
    int val = analogRead(ZMPT_PIN);
    if (val > v_max) v_max = val;
    if (val < v_min) v_min = val;
    sum_raw += val;
    n_samples++;
    delayMicroseconds(200);
  }

  // Baseline (DC offset) = rata-rata
  float baseline = (float)sum_raw / n_samples;

  // Hitung RMS dari sampel (perlu second pass — pakai rata-rata sebagai acuan)
  // Karena sampel sudah lewat, kita hitung ulang dengan sampling kedua singkat
  sum_sqr = 0;
  n_samples = 0;
  start_time = millis();
  while (millis() - start_time < 1000) {
    int val = analogRead(ZMPT_PIN);
    int offset = val - (int)baseline;
    sum_sqr += (long)offset * offset;
    n_samples++;
    delayMicroseconds(200);
  }
  float rms = sqrt((float)sum_sqr / n_samples);

  int v_mid = (v_max + v_min) / 2;
  int v_pp  = v_max - v_min;  // Peak-to-Peak

  // ── Tampilan hasil ─────────────────────────────────────────────────────────
  Serial.println();
  Serial.printf("Min: %4d | Max: %4d | Mid: %4d | Vpp: %4d | Base: %.0f | RMS: %.1f\n",
                v_min, v_max, v_mid, v_pp, baseline, rms);
  Serial.print("STATUS: ");

  // ── Analisis kondisi gelombang ─────────────────────────────────────────────
  if (v_max >= 3900 || v_min <= 150) {
    Serial.println("[CLIPPING / OVERGAIN] Putar baut CCW (berlawanan jarum jam)!");
  }
  else if (v_pp < 800) {
    Serial.println("[GAIN TERLALU RENDAH] Putar baut CW (searah jarum jam)!");
  }
  else if (v_mid < 1500 || v_mid > 2600) {
    Serial.printf("[MIDPOINT MIRING] Mid=%d (target ~2048). Cek DC offset / supply.\n", v_mid);
  }
  else {
    Serial.println("[PAS / IDEAL] Kunci posisi baut, siap kalibrasi kodingan.");
  }

  Serial.println("----------------------------------------------");
  delay(500);
}
