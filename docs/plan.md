# Sistem Kontrol Pencahayaan Nirkabel Buah Naga - Plan

## Overview
This project develops an IoT lighting control system for dragon fruit cultivation with real-time energy monitoring and Firebase integration.

## Goals
- Real-time energy monitoring via PZEM-004T sensors
- Remote control via Firebase RTDB
- Local status display via OLED
- Energy efficiency and safety optimization

## Tasks
1. **Hardware Setup**
   - Verify ESP32 connections (SSR, PZEM, OLED)
   - Confirm wiring diagrams match hardware layout

2. **Firmware Development**
   - Implement ESP32 WiFi Manager (OTM) for OTA setup
   - Integrate PZEM-004T sensors with UART (3.3V logic)
   - Implement Firebase RTDB for data storage and control
   - Use LED indicators for real-time status feedback

3. **Testing**
   - Validate WiFi connectivity and OTA updates
   - Test sensor readings and relay control
   - Verify Firebase data synchronization and command handling

4. **Documentation**
   - Update README.md with new setup instructions
   - Document wiring diagrams and code structure

## Next Steps
- Complete firmware implementation with Firebase integration
- Test all components sequentially
- Finalize documentation and prepare for PR