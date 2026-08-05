# PZEM-004T v3.0 IoT Lighting Control Sensor Integration

## Overview
This document describes the wiring, configuration, and troubleshooting for integrating the PZEM-004T v3.0 energy monitoring module with ESP32 firmware for the SmartNaga lighting control system.

## Hardware Requirements
- **ESP32 DevKit 38-pin** (e.g., ESP32 DevKit 38-pin, Wemos D1 R32)
- **PZEM-004T v3.0** energy meter
- **Jumper wires (male-to-male)**
- **Power source** (220-240V AC) for PZEM input
- Optional: Solid State Relay (SSR) for lamp control

## Wiring Diagram

```
PZEM-004T v3.0          ESP32 DevKit 38-pin
┌─────────────┐         ┌─────────────────────┐
│ VCC (5V)    │ ──────►  │ 5V (or 3.3V)        │
│ GND         │ ──────►  │ GND                 │
│ TX          │ ──────►  │ GPIO32 (TX2)        │
│ RX          │ ──────►  │ GPIO33 (RX2)      │
└─────────────┘         └─────────────────────┘
```

**Important Notes:**
- **Power Supply**: PZEM-004T v3.0 requires AC power (L and N terminals) to operate. The module does not use external VCC/GND for power.
- **Grounding**: Ensure the PZEM GND is connected to ESP32 GND for proper UART communication.
- **Wiring Order**: 
  - PZEM TX → ESP32 GPIO33 (RX2)
  - PZEM RX → ESP32 GPIO32 (TX2)
  - Common Ground: Connect PZEM GND to ESP32 GND

## Pin Mapping (ESP32)

| Function          | ESP32 Pin | Description |
|-------------------|-----------|-------------|
| PZEM TX         | GPIO32    | Sends data to PZEM RX |
| PZEM RX         | GPIO33    | Receives data from PZEM TX |
| SSR Control (A)   | GPIO13    | Drives SSR (active HIGH) |
| SSR Control (B)   | GPIO12    | Future expansion |
| Physical Button   | GPIO4     | Toggles lamp A state (active LOW) |
| LED Indicator     | GPIO2     | Built-in LED (active LOW) |

## Why GPIO16/17 Failed & SoftwareSerial Not Working

### GPIO16/17 Failure Analysis
- **Root Cause**: GPIO16 and GPIO17 on ESP32 38-pin boards are **shared with PSRAM** (Pseudo-Static RAM). When a board has PSRAM installed (which is common on ESP32 DevKit 38-pin / Wemos D1 R32), these pins are used for PSRAM interface (SPI flash/PSRAM communication).
- **Result**: These pins cannot be repurposed for UART (Serial2) without disabling PSRAM, which would break many Arduino libraries that depend on PSRAM (including ArduinoJson, WiFiManager, etc.).
- **Solution**: Use alternative UART-capable GPIOs that don't conflict with PSRAM — **GPIO32 (TX2) and GPIO33 (RX2)** are standard replacements.

### SoftwareSerial Failure Analysis
- **Root Cause**: ESP32 Arduino core (based on ESP-IDF) **does not include `SoftwareSerial.h`** in the standard library path. The `SoftwareSerial` library is designed for AVR-based boards (Arduino Uno, Mega, Nano) that lack multiple hardware UARTs.
- **Why**: ESP32 has **3 hardware UARTs** (Serial0, Serial1, Serial2) natively supported by the ESP-IDF framework. There's no need for software-based UART emulation, and implementing it on dual-core ESP32 with FreeRTOS is complex.
- **Solution**: Always use **HardwareSerial** (Serial, Serial1, or Serial2) on ESP32. All three are fully functional with configurable pins.

## Troubleshooting Guide
- **AC Input**: 220-240V AC across L (Live) and N (Neutral) terminals.
- **Module Power**: The PZEM draws power from the AC line via its internal supply; no separate VCC/GND connection required.
- **Logic Level**: 3.3V TTL logic levels (compatible with ESP32).

## Troubleshooting Guide

| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| `Address: 0x00` | No AC power to L/N, or wiring error | Verify AC connection to L/N terminals; check wiring |
| `Gagal baca data Blok A` | Incorrect wiring (TX/RX swapped), no common ground, or no AC power | Verify wiring: TX→GPIO33, RX→GPIO32, GND common; ensure AC power is connected |
| No data on Serial Monitor | Serial2 not initialized or baud rate mismatch | Ensure `Serial2.begin(9600, SERIAL_8N1, 32, 33)` is called in `setup()` |
| `nan` values | Unstable power or poor connections | Verify AC connection to L/N, check wiring tightness |
| No response at all | Incorrect baud rate or wrong UART port | Verify 9600 baud rate and correct UART pins (GPIO32/33) |

## Common Issues & Fixes

- **No response from PZEM**: Verify AC power is connected to L and N terminals. LED on PZEM should be lit.
- **Garbage or random values**: Check wiring order (TX↔RX) and ensure common ground between ESP32 and PZEM.
- **Communication timeout**: Increase `timerPZEM.interval` to allow more time for PZEM response (default 5000ms).
- **Firmware version mismatch**: Ensure PZEM library version matches the module (v3.0 as used in this project).
- **GPIO16/17 failure**: These pins are shared with PSRAM on 38-pin boards. Use GPIO32/33 instead.
- **SoftwareSerial missing**: ESP32 Arduino core does not include `SoftwareSerial.h`. Always use HardwareSerial (Serial/Serial1/Serial2).

## Integration Notes

- The firmware uses **non-blocking timers** (`millis()`) for PZEM reading (every 5s) and Firebase updates (every 10s).
- Power readings are stored in an `EnergyData` struct and sent to Firebase RTDB at `/energy/block_a`.
- The app listens to both `/lamp/control/{a,b}` (commands) and `/lamp/status/{a,b}` (feedback) for real-time sync.

## Notes
- **Power Anomaly Detection**: Compare actual power readings against expected values per channel. Implement threshold-based alerts (e.g., >10% deviation for >30s).
- **Hybrid Detection**: Combine app-side real-time monitoring with a Cloud Function that runs 24/7 for background anomaly detection.
- **Debounce Strategy**: Wait 30 seconds after ON toggle before comparing power readings to avoid false alarms during startup transients.