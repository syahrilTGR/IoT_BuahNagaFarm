# WIRING — ESP32 DevKit 38-Pin

**Sistem Kontrol Pencahayaan Nirkabel Buah Naga**

---

## Komponen Utama

| #  | Komponen                | Qty | Keterangan                        |
|----|-------------------------|-----|-----------------------------------|
| 1  | ESP32 DevKit 38-Pin     | 1   | ESP32-WROOM-32, 3.3V logic        |
| 2  | PZEM-004T v3.0         | 1   | Energy meter, 3.3V UART           |
| 3  | SSR (Solid State Relay)| 1   | Zero-cross, 24VDC input, 40A     |
| 4  | LLC (Logic Level Converter) | 1 | 3.3V → 5V bidirectional       |
| 5  | Power Supply 5V 2A     | 1   | Untuk SSR input + LLC             |
| 6  | Kabel jumper            | -   | Male-female, male-male            |

---

## GPIO Mapping

| Fungsi            | GPIO | Pin Fisik | Arah     | Catatan                     |
|-------------------|------|-----------|----------|-----------------------------|
| PZEM RX           | 16   | D16/RX2   | ESP32 ←  | UART2 receive               |
| PZEM TX           | 17   | D17/TX2   | ESP32 →  | UART2 transmit              |
| SSR Control       | 13   | D13       | ESP32 →  | Via LLC 3.3V→5V → SSR IN    |
| Built-in LED      | 2    | D2        | ESP32 →  | Active LOW (test only)      |
| Button Test       | 4    | D4        | ESP32 ←  | INPUT_PULLUP (test only)    |
| VCC               | -    | 3.3V      | ESP32 →  | PZEM VCC (3.3V langsung)    |
| GND               | -    | GND       | ESP32 ←  | Common ground semua komponen|

---

## Wiring Diagram

```
┌─────────────────────────────────────────────────┐
│                  ESP32 DevKit 38-Pin             │
│                                                  │
│  ┌──────────────────────────────────────────┐   │
│  │  D16 (RX2) ──────────── TX (PZEM)       │   │
│  │  D17 (TX2) ──────────── RX (PZEM)       │   │
│  │  D13       ───► LLC IN  ───► SSR IN      │   │
│  │  3.3V      ──────────── VCC (PZEM)       │   │
│  │  GND       ──────────── GND (common)     │   │
│  └──────────────────────────────────────────┘   │
└─────────────────────────────────────────────────┘
         │                                    │
         ▼                                    ▼
┌─────────────────┐                 ┌─────────────────┐
│  PZEM-004T v3   │                 │  LLC (3.3V↔5V)  │
│  ┌──────────┐   │                 │  IN  ──► OUT    │
│  │ TX  RX   │   │                 │  3.3V ──► 5V    │
│  │ 3.3V GND │   │                 └────────┬────────┘
│  └──────────┘   │                          │
└────────┬────────┘                          ▼
         │                           ┌─────────────────┐
         │                           │  SSR (24VDC IN)  │
         │                           │  IN  ← LLC OUT  │
         │                           │  LOAD ← AC MAIN │
         │                           └────────┬────────┘
         │                                    │
         └──────────────┬─────────────────────┘
                        │
                   ┌────┴────┐
                   │   GND   │
                   └─────────┘
```

---

## Wiring PZEM-004T ke ESP32

PZEM-004T v3.0 memiliki UART 3.3V logic — **langsung connect ke ESP32 tanpa LLC.**

```
PZEM-004T v3.0          ESP32
──────────────          ─────
TX  ──────────────►    D16 (RX2)
RX  ◄──────────────    D17 (TX2)
VCC ──────────────►    3.3V
GND ──────────────►    GND
```

**Catatan:**
- Jangan hubungkan ke 5V → akan merusak ESP32 GPIO
- Pastikan GND PZEM dan ESP32 tersambung (common ground)

---

## Wiring SSR via LLC

ESP32 GPIO 13 (3.3V) → LLC → SSR IN (5V/24VDC)

```
ESP32               LLC                  SSR
─────               ───                  ───
D13 ────────►  HV1 (3.3V IN)  ──►  LV1 (5V OUT)
3.3V ────────►  HV VCC
GND ─────────►  HV GND         ──►  LV GND
5V  ──────────►                 ──►  LV VCC
                                   │
                                   ▼
                              SSR DC IN (+/-)
                              SSR LOAD ← AC MAINS
```

**LLC Level Converter:**
- HV side: 3.3V dari ESP32
- LV side: 5V dari power supply
- Direction: HV1 → LV1 (ESP32 → SSR)

**SSR Connection:**
- DC Input: 5V/24VDC (dari LLC output)
- AC Load: Terhubung ke lampu + AC mains
- ⚠️ **HATI-HATI: AC mains 220V berbahaya!**

---

## Power Distribution

```
┌─────────────────────────────────────────────┐
│              Power Supply 5V/2A             │
│                                             │
│  5V  ────────► LLC LV VCC                   │
│  GND ────────► LLC LV GND                   │
│                LLC HV GND ──────► ESP32 GND │
│                                             │
│  (Jika SSR butuh 24VDC: tambah converter)   │
└─────────────────────────────────────────────┘

┌─────────────────────────────────────────────┐
│              ESP32 USB / External 5V        │
│                                             │
│  5V  ────────► ESP32 VIN (jika pakai 5V)    │
│  GND ────────► ESP32 GND                    │
│                ────────► PZEM GND           │
│                                             │
│  3.3V ───────► PZEM VCC                     │
└─────────────────────────────────────────────┘
```

---

## ⚠️ Safety Notes

1. **Jangan sentuh wiring AC saat listrik hidup**
2. **Gunakan multimeter untuk cek voltage sebelum connect**
3. **SSR Zero-cross** hanya cocok untuk resistive load (lampu)
4. **Isolasi optocoupler di SSR** melindungi ESP32 dari HV
5. **Test dengan multimeter dulu** sebelum hubungkan ke lampu
6. **Ganti dengan SSR rating yang sesuai** untuk jumlah lampu

---

## Physical Pin Reference (ESP32 DevKit 38-Pin)

```
         ESP32 DevKit 38-Pin
    ┌──────────────────────────┐
    │  [GND] [GPIO23] [GPIO22]│ ← Top Row
    │  [GPIO1] [GPIO3]  [RX2] │
    │  [TX2]  [GPIO21] [GND]  │
    │  [GPIO19] [GPIO18][GPIO5]│
    │  [GPIO17] [GPIO16][GPIO4]│
    │  [GPIO0]  [GPIO2] [GPIO15]│
    │  [GPIO13] [GPIO12][GND]  │
    │  [GPIO14] [GPIO27][GPIO26]│ ← Bottom Row
    │  [GPIO25] [GPIO33][GPIO32]│
    │  [VIN]   [3.3V]  [EN]    │
    └──────────────────────────┘
         USB
```

**Pin yang digunakan:**
- GPIO16 (RX2) — PZEM TX
- GPIO17 (TX2) — PZEM RX
- GPIO13 (D13) — SSR Control
- GPIO2  (D2)  — Built-in LED (test)
- GPIO4  (D4)  — Button (test)
- 3.3V   — PZEM VCC
- GND    — Common ground

---

*Dokumen ini sesuai untuk ESP32 DevKit 38-pin. Verifikasi pinout sebelum wiring!*