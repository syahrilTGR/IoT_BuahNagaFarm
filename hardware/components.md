# Hardware Components Reference

## ESP32 DevKit V1
- GPIO UART: TX0=GPIO1, RX0=GPIO3 (untuk PZEM)
- GPIO I2C: SDA=GPIO21, SCL=GPIO22 (untuk OLED)
- PWM: GPIO25-27 (untuk relay control)
- Harga: Rp 250.000-500.000

## PZEM-004T v3.0 (x2)
- Komunikasi: UART (1 = TX, 2 = RX) - gunakan SoftwareSerial
- Library: `mandulaj/PZEM-004T-v30`
- CT: 100A eksternal, diklem di kabel fasa ke beban
- Harga: Rp 199.000 - 299.000 per unit

## Logic Level Converter (4-channel BSS138)
- Channel: 4 (cukup untuk UART + I2C)
- LV: 3.3V (ESP32), HV: 5V (PZEM/OLED)
- Harga: Rp 8.000-15.000

## AC Detector (Optocoupler)
- Opsi 1: SCT-013-000 (current transformer)
- Opsi 2: H11AA1 / 4N35 (optocoupler standar)
- Output: 3.3V logic level

## OLED LCD 0.96" 128x64
- I2C address: 0x3C (default) / 0x3D
- Library: `Adafruit SSD1306`
- Harga: Rp 25.000-45.000

## Solid State Relay (SSR) 2-Channel
- Input: 3.3V/5V DC trigger = ON
- Output: 220V AC, 10A rating
- Harga: Rp 15.000-25.000 per channel

## Power Supply + Step Down
- LM2596 buck: Input 12V, Output 5V 3A
- Harga: Rp 8.000-15.000
- UPS board 12V (optional): Rp 25.000-50.000

## USB Modem LTE (Opsional)
- Alternatif: gunakan WiFi ESP32 built-in
- Tipe: Huawei E3372 (kalau mau konektivitas LTE backup)