# MANUAL BOOK & PANDUAN TEKNIS
## SMARTNAGA: SISTEM KONTROL PENCAHAYAAN NIRKABEL & MONITORING ENERGI BUAH NAGA BERBASIS IoT

---

```
  ███████╗███╗   ███╗ █████╗ ██████╗ ████████╗███╗   ██╗ █████╗  ██████╗  █████╗ 
  ██╔════╝████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝████╗  ██║██╔══██╗██╔════╝ ██╔══██╗
  ███████╗██╔████╔██║███████║██████╔╝   ██║   ██╔██╗ ██║███████║██║  ███╗███████║
  ╚════██║██║╚██╔╝██║██╔══██║██╔══██╗   ██║   ██║╚██╗██║██╔══██║██║   ██║██╔══██║
  ███████║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║   ██║ ╚████║██║  ██║╚██████╔╝██║  ██║
  ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝
             Sistem Smart Farming IoT untuk Kebun Buah Naga Off-Season
```

**Versi Dokumen:** 2.0  
**Tahun Rilis:** 2026  
**Target Pengguna:** Pengelola Kebun, Teknisi Lapangan, Peneliti/Akademisi  
**Penyusun:** Tim Riset & Magang IoT Smart Farming Buah Naga  

---

## DAFTAR ISI

1. [BAB I: PENDAHULUAN & GAMBARAN UMUM SISTEM](#bab-i-pendahuluan--gambaran-umum-sistem)
   - 1.1 Latar Belakang & Fungsi Alat
   - 1.2 Arsitektur Sistem 3-Tier (Edge, Cloud, App)
   - 1.3 Fitur-Fitur Utama Sistem
   - 1.4 Spesifikasi Teknis Perangkat
2. [BAB II: DAFTAR KOMPONEN & SPESIFIKASI HARDWARE](#bab-ii-daftar-komponen--spesifikasi-hardware)
   - 2.1 Bill of Materials (BOM)
   - 2.2 Deskripsi Fungsi Tiap Modul
   - 2.3 Skema Distribusi Catu Daya (Dual-Power Architecture)
3. [BAB III: SKEMA RANGKAIAN & PANDUAN PENGKABELAN (WIRING)](#bab-iii-skema-rangkaian--panduan-pengkabelan-wiring)
   - 3.1 Pemetaan Pinout ESP32 DevKit 38-Pin
   - 3.2 Diagram Koneksi Sensor Energi PZEM-004T (Dual UART)
   - 3.3 Diagram Sensor Tegangan PLN ZMPT101B
   - 3.4 Diagram Bus I2C (OLED SSD1306 & RTC DS3231)
   - 3.5 Diagram Aktuator Relay & Kontaktor Lampu
   - 3.6 Diagram Alarm Buzzer
4. [BAB IV: INSTALASI FISIK & KESELAMATAN KERJA (K3)](#bab-iv-instalasi-fisik--keselamatan-kerja-k3)
   - 4.1 Standar Keamanan Tegangan Tinggi (AC 220V)
   - 4.2 Tata Letak Box Panel & Terminal Blok
   - 4.3 Pemasangan Current Transformer (CT) di Lapangan
   - 4.4 Proteksi Cuaca (Waterproofing & Suhu Tropis)
5. [BAB V: PANDUAN PENGATURAN AWAL & KONEKTIVITAS (WIFIMANAGER)](#bab-v-panduan-pengaturan-awal--konektivitas-wifimanager)
   - 5.1 Mode Hotspot Konfigurasi (SmartNaga-AP)
   - 5.2 Langkah-Langkah Menghubungkan ke WiFi / Modem 4G
   - 5.3 Indikator Status Jaringan & Ping Latency Test
   - 5.4 Prosedur Reset Kredensial WiFi Jarak Jauh
6. [BAB VI: ANTARMUKA LOKAL & PENGOPERASIAN DISPLAY OLED](#bab-vi-antarmuka-lokal--pengoperasian-display-oled)
   - 6.1 Struktur Tampilan Layar 7-Baris
   - 6.2 Arti Status & Notifikasi Visual
   - 6.3 Indikator Suara (Audio Alert / Buzzer)
   - 6.4 Mode Operasi Offline (Fail-Safe RTC DS3231)
7. [BAB VII: LOGIKA DIAGNOSA CERDAS (DECISION TREE 10 STATUS)](#bab-vii-logika-diagnosa-cerdas-decision-tree-10-status)
   - 7.1 Filosofi Firmware-First Diagnostic
   - 7.2 Diagram Alir Evaluasi Status Beban
   - 7.3 Penjelasan 10 Status Diagnosa Lapangan
   - 7.4 Mekanisme Estimasi Jumlah Lampu Putus
8. [BAB VIII: INTEGRASI CLOUD & APLIKASI MOBILE](#bab-viii-integrasi-cloud--aplikasi-mobile)
   - 8.1 Sinkronisasi Firebase Realtime Database
   - 8.2 Struktur Payload Telemetri & Kontrol
   - 8.3 Fitur Kontrol Manual & Penjadwalan Lampu
   - 8.4 Konfigurasi Parameter Beban (Expected Power & Toleransi)
9. [BAB IX: KALIBRASI SENSOR & PENGUJIAN AKURASI](#bab-ix-kalibrasi-sensor--pengujian-akurasi)
   - 9.1 Kalibrasi Sensor Tegangan ZMPT101B
   - 9.2 Kalibrasi Sensor PZEM-004T v3.0
   - 9.3 Validasi Akurasi Waktu RTC DS3231 via NTP
10. [BAB X: PANDUAN PEMELIHARAAN & PEMECAHAN MASALAH (TROUBLESHOOTING)](#bab-x-panduan-pemeliharaan--pemecahan-masalah-troubleshooting)
    - 10.1 Tabel Matriks Masalah & Solusi Cepat
    - 10.2 Pemeliharaan Preventif Bulanan & Musiman
    - 10.3 Prosedur Pembaruan Firmware (USB PlatformIO & OTA WiFi)

---

## BAB I: PENDAHULUAN & GAMBARAN UMUM SISTEM

### 1.1 Latar Belakang & Fungsi Alat
Tanaman buah naga (*Hylocereus polyrhizus* / *Selenicereus undatus*) merupakan tanaman hari panjang (*long-day plant*) yang membutuhkan induksi pencahayaan malam hari (fotoperiodisme) selama 4 hingga 6 jam untuk merangsang diferensiasi kuncup vegetatif menjadi kuncup bunga di luar musim (*off-season*).

Pencahayaan buatan skala kebun memerlukan konsumsi energi listrik yang cukup tinggi dan rawan mengalami kendala:
1. **Lampu putus sebagian/total tanpa diketahui** sehingga induksi gagal dan bunga tidak mekar merata.
2. **Pemadaman listrik PLN tiba-tiba** di area perkebunan terpencil tanpa notifikasi seketika ke petani.
3. **Fluktuasi tegangan ekstrim** yang membakar trafo lampu atau memperpendek usia lampu LED.
4. **Relay pengontrol lengket/bocor (*contact weld*)** yang menyebabkan lampu menyala terus-menerus dan boros listrik.

**SmartNaga** dirancang sebagai sistem kontrol pencahayaan nirkabel dan pengawasan energi cerdas (*Smart Energy Management & Diagnostics*) yang dapat bekerja otomatis, mendeteksi kerusakan secara mandiri menggunakan algoritma pohon keputusan (*decision tree*), serta dapat dipantau dan dikendalikan jarak jauh melalui internet menggunakan aplikasi smartphone Android/iOS.

---

### 1.2 Arsitektur Sistem 3-Tier

Sistem SmartNaga mengusung arsitektur terintegrasi yang terbagi dalam 3 tingkatan utama:

```
┌────────────────────────────────────────────────────────────────────────┐
│                        USER TIER (APLIKASI MOBILE)                     │
│    Flutter SmartNaga App: Kontrol Saklar, Jadwal, Monitoring Energi,   │
│           Konfigurasi Lampu Kebun, Log Notifikasi Gangguan             │
└───────────────────────────────────▲────────────────────────────────────┘
                                    │ HTTPS / WebSocket (SSL/TLS)
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                          CLOUD TIER (BACKEND)                          │
│     Firebase Realtime Database (RTDB)  ─── Telemetri & Kontrol Nyata   │
│     Cloud Firestore                    ─── Histori Sesi & Rekam Jejak  │
└───────────────────────────────────▲────────────────────────────────────┘
                                    │ MQTT / RTDB REST over WiFi 2.4GHz
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                       EDGE TIER (KONTROLER ESP32)                      │
│  ┌───────────────────────┐ ┌────────────────────┐ ┌──────────────────┐ │
│  │   ESP32 Main Core     │ │   Decision Tree    │ │   WiFi Manager   │ │
│  │ (FreeRTOS Dual-Core)  │ │ (10 Status Cerdas) │ │  (Portal On-Fly) │ │
│  └───────────┬───────────┘ └─────────┬──────────┘ └────────┬─────────┘ │
│              │                       │                     │           │
│  ┌───────────▼───────────┐ ┌─────────▼──────────┐ ┌────────▼─────────┐ │
│  │ Sensor Energi         │ │ Tampilan & Waktu   │ │ Aktuator Beban   │ │
│  │ • 2x PZEM-004T v3.0   │ │ • OLED SSD1306 I2C │ │ • Relay SSR Dual │ │
│  │ • 1x ZMPT101B Voltage │ │ • RTC DS3231 I2C   │ │ • Active Buzzer  │ │
│  └───────────────────────┘ └────────────────────┘ └──────────────────┘ │
└────────────────────────────────────────────────────────────────────────┘
```

---

### 1.3 Fitur-Fitur Utama Sistem

* **Kontrol Nirkabel Dua Blok:** Pengendalian terpisah untuk Blok A dan Blok B secara manual via smartphone maupun otomatis via jadwal kalender.
* **Dual Monitoring Energi Presisi:** Pengukuran Tegangan (V), Arus (A), Daya Aktif (W), dan Konsumsi Energi Akumulatif (kWh) per masing-masing blok secara independen via PZEM-004T v3.0.
* **Deteksi Sumber Listrik Seketika:** Modul ZMPT101B mendeteksi seketika apakah jalur utama menggunakan suplai PLN atau beralih ke cadangan UPS/Inverter.
* **Firmware-First Decision Tree:** Algoritma pemantau beban berjalan 24/7 di level firmware ESP32; status kerusakan langsung tampil di layar lokal box kebun meskipun koneksi internet terputus.
* **Estimasi Jumlah Lampu Rusak:** Jika beberapa bohlam putus, sistem langsung mengkalkulasikan dan menampilkan perbandingan lampu menyala (contoh: `Status: 10/12 Normal`).
* **Sistem Suplai Tahan Padam (Dual Power):** Kontroler dan modem dipasok melalui UPS, sehingga saat PLN padam total, sistem tetap hidup dan seketika mengirimkan sinyal darurat (*Push Notification*) ke pemilik.
* **Konfigurasi WiFi Mandiri (OTM):** Tanpa perlu memprogram ulang kode mikrokontroler, penggantian SSID/Password WiFi kebun dapat dilakukan melalui captive portal web browser smartphone.
* **Indikator Audio & Visual:** Dilengkapi layar OLED 0.96 inci resolusi tinggi dan active buzzer alarm untuk kemudahan inspeksi langsung oleh petani di lapangan.

---

### 1.4 Spesifikasi Teknis Perangkat

| Kategori | Parameter | Nilai Spesifikasi |
|---|---|---|
| **Kontroler** | Mikrokontroler | ESP32-WROOM-32 (Dual-Core 32-bit Tensilica LX6, 240 MHz) |
| | Memori | 520 KB SRAM, 4 MB Flash |
| **Konektivitas** | Nirkabel | Wi-Fi 802.11 b/g/n (2.4 GHz) |
| | Protokol Cloud | Firebase REST / WebSocket API dengan Auth Email-Password |
| | Diagnostik Koneksi | ICMP Ping (8.8.8.8) non-blocking interval 3 detik |
| **Pengukuran Listrik** | Rentang Tegangan AC | 80 ~ 260 V AC (Resolusi: 0.1 V, Akurasi: 0.5%) |
| | Rentang Arus AC | 0 ~ 100 A (Resolusi: 0.001 A, Akurasi: 0.5%) |
| | Rentang Daya Aktif | 0 ~ 23 kW (Resolusi: 0.1 W, Akurasi: 0.5%) |
| | Akumulasi Energi | 0 ~ 9999.99 kWh (Resolusi: 1 Wh) |
| | Sampling Deteksi PLN | 600 Sampel per cycle (~48 ms) dengan filter RMS |
| **Display & Waktu** | Layar Lokal | OLED 0.96" 128x64 Piksel, Driver SSD1306 (Monochrome) |
| | Real-Time Clock | DS3231 SN High-Precision TCXO RTC (Akurasi: ±2 ppm) |
| **Kelistrikan & Daya** | Sumber Daya Kontrol | 220V AC via UPS → Adaptor 12V 5A → Buck Converter 5V 3A |
| | Konsumsi Daya Node | ~1.5 - 3.5 Watt (ESP32 + OLED + Sensor + Relay Active) |
| | Tegangan Kerja Logika | 3.3V DC (ESP32 / Sensor I2C) & 5V DC (Relay / ZMPT) |
| | Kapasitas Relay Beban | Solid State Relay / Kontaktor hingga 25A - 40A per Blok |
| **Lingkungan Kerja** | Suhu Operasional | -10°C sampai +65°C |
| | Ketahanan Box Panel | Enclosure IP65 Tahan Debu dan Cipratan Air Hujan |

---

## BAB II: DAFTAR KOMPONEN & SPESIFIKASI HARDWARE

### 2.1 Bill of Materials (BOM)

| No | Nama Komponen / Modul | Tipe / Model | Jumlah | Keterangan |
|---|---|---|:---:|---|
| 1 | Modul Mikrokontroler | ESP32 DevKit V1 (38 Pin) | 1 unit | Otak kontroler utama |
| 2 | Sensor Pemantau Energi | PZEM-004T v3.0 + CT 100A | 2 unit | Pengukur daya Blok A & Blok B |
| 3 | Sensor Deteksi Tegangan AC | ZMPT101B Module | 1 unit | Deteksi keberadaan listrik PLN |
| 4 | Modul Real-Time Clock | RTC DS3231 I2C + Baterai CR2032 | 1 unit | Pewaktu presisi tinggi offline |
| 5 | Layar Tampilan Status | OLED 0.96" I2C SSD1306 (128x64) | 1 unit | Display monitoring box lokal |
| 6 | Aktuator Kontrol Lampu | Relay Modul 2 Channel / SSR | 1 unit | Pemicu koil kontaktor AC 220V |
| 7 | Kontaktor Magnetik AC | Schneider / Chint 220V 25A | 2 unit | Saklar daya beban lampu kebun |
| 8 | Modul Konversi Logika | Logic Level Converter 4-Ch (BSS138)| 1 unit | Konversi sinyal 3.3V ↔ 5V |
| 9 | Alarm Suara | Active Buzzer 5V DC | 1 unit | Indikator audio alert |
| 10 | Sumber Daya Cadangan | Mini UPS AC 220V / Baterai Backup | 1 unit | Penopang daya kontroler & modem |
| 11 | Power Supply Utama | Switching Power Supply 12V 5A | 1 unit | Konversi AC 220V ke DC 12V |
| 12 | Step-Down Regulator | Buck Converter LM2596 5V 3A | 1 unit | Penurun tegangan 12V ke 5V DC |
| 13 | Terminal Blok | Terminal Block Strip 12 Pin | 2 unit | Distribusi perkabelan rapi |
| 14 | Mini MCB Proteksi | MCB 1-Phase 10A / 16A | 2 unit | Proteksi arus hubung singkat |
| 15 | Kotak Panel Pelindung | Enclosure Panel Box Outdoor IP65 | 1 unit | Perlindungan cuaca perkebunan |

---

### 2.2 Deskripsi Fungsi Tiap Modul

```
 ┌─────────────────┐       ┌─────────────────┐       ┌─────────────────┐
 │ ESP32 DEVKIT V1 │       │ PZEM-004T V3.0  │       │  ZMPT101B MODUL │
 │ Otak sistem     │       │ Metering volt,  │       │ Deteksi instan  │
 │ pemroses data & │       │ arus, watt &    │       │ listrik PLN     │
 │ gateway WiFi    │       │ kWh beban kebun │       │ vs suplai UPS   │
 └─────────────────┘       └─────────────────┘       └─────────────────┘
 ┌─────────────────┐       ┌─────────────────┐       ┌─────────────────┐
 │   OLED SSD1306  │       │   RTC DS3231    │       │ SOLID STATE/SSR │
 │ Monitor lokal   │       │ Jam akurat RTC  │       │ Saklar beban AC │
 │ 7-baris di box  │       │ sinkron NTP     │       │ tanpa percikan  │
 └─────────────────┘       └─────────────────┘       └─────────────────┘
```

1. **ESP32 DevKit V1:** Mengatur pembacaan berkala seluruh sensor, mengeksekusi pohon keputusan diagnosa status lampu setiap 1 detik, merespon saklar relay, dan berkomunikasi dengan Firebase.
2. **PZEM-004T v3.0 (2 Unit):** Modul pengukur parameter listrik berstandar industri dengan isolasi optocoupler bawaan. Terkoneksi menggunakan komunikasi serial UART (UART1 untuk Blok B, UART2 untuk Blok A).
3. **ZMPT101B Voltage Transformer Module:** Memiliki potensiometer penyesuaian gain. Digunakan untuk membaca gelombang sinus AC jala-jala PLN guna membedakan pemadaman PLN dari kerusakan kabel lokal.
4. **RTC DS3231:** Menjamin pencatatan waktu operasional dan eksekusi jadwal penyalaan lampu tetap tepat detik meski alat kehilangan koneksi internet atau router mati.
5. **OLED SSD1306 0.96":** Menampilkan visualisasi data langsung di depan box panel kebun tanpa perlu teknisi mengeluarkan HP atau laptop.
6. **Relay & Kontaktor Magnetik:** Mengisolasi mikrokontroler dari beban induktif dan resistif ratusan watt lampu kebun, mencegah lonjakan arus balik (*back-EMF*).

---

### 2.3 Skema Distribusi Catu Daya (Dual-Power Architecture)

Salah satu keunggulan utama sistem SmartNaga adalah **pemisahan jalur daya beban dan jalur daya kontrol**:

```
                       ┌─────────────────────────┐
                       │   SUMBER DAYA PLN 220V  │
                       └────────────┬────────────┘
                                    │
           ┌────────────────────────┴────────────────────────┐
           │                                                 │
           ▼ Jalur Beban (TIDAK LEWAT UPS)                   ▼ Jalur Kontrol (LEWAT UPS)
 ┌───────────────────┐                             ┌───────────────────┐
 │  MCB Beban 16A    │                             │  Mini UPS 220V    │
 └─────────┬─────────┘                             └─────────┬─────────┘
           │                                                 │
 ┌─────────▼─────────┐                             ┌─────────▼─────────┐
 │ ZMPT101B (Deteksi)│                             │ Power Supply 12V  │
 └─────────┬─────────┘                             └─────────┬─────────┘
           │                                                 │
 ┌─────────▼─────────┐                             ┌─────────▼─────────┐
 │ Kontaktor A & B   │                             │ Step-Down LM2596  │
 └─────────┬─────────┘                             │ Output 5.0V DC    │
           │                                       └─────────┬─────────┘
 ┌─────────▼─────────┐                                       │
 │ Lampu Blok A & B  │                       ┌───────────────┴───────────────┐
 └───────────────────┘                       │                               │
                                             ▼                               ▼
                                   ┌───────────────────┐           ┌───────────────────┐
                                   │ ESP32 VIN (5V)    │           │ Modem WiFi 4G LTE │
                                   │ PZEM VCC / OLED   │           │ (Internet Backup) │
                                   └───────────────────┘           └───────────────────┘
```

> [!IMPORTANT]
> **Mengapa Jalur Kontrol Harus Menggunakan UPS?**  
> Jika seluruh sistem mati saat PLN padam, mikrokontroler akan langsung *down* dan tidak sempat mengirimkan data status ke Firebase. Dengan arsitektur di atas, saat PLN padam:
> 1. Lampu kebun padam seketika (beban tidak menguras baterai UPS).
> 2. ZMPT101B mendeteksi tegangan drop ke 0V.
> 3. ESP32 tetap menyala normal melalui daya baterai UPS.
> 4. Buzzer lokal membunyikan alarm panjang selama 2 detik.
> 5. ESP32 mengirimkan notifikasi telemetri `/energy/pln_status` = `{"available": false}` ke cloud.
> 6. Aplikasi di smartphone petani menerima peringatan: *"PERINGATAN: Listrik PLN Padam!"*

---

## BAB III: SKEMA RANGKAIAN & PANDUAN PENGKABELAN (WIRING)

### 3.1 Pemetaan Pinout ESP32 DevKit 38-Pin

Berikut adalah tabel koneksi fisik seluruh pin ESP32 yang digunakan pada firmware produksi SmartNaga (`src/main.cpp`):

| Pin ESP32 | Tipe GPIO | Nama Modul | Pin Modul | Deskripsi Fungsi |
|---|---|---|---|---|
| **GPIO13** | Output | Relay Modul | IN 1 (Blok A) | Kendali Relay Lampu Blok A (Active LOW) |
| **GPIO14** | Output | Relay Modul | IN 2 (Blok B) | Kendali Relay Lampu Blok B (Active LOW) |
| **GPIO33** | Input | PZEM-004T (A) | TX | Serial2 RX: Terima data energi Blok A |
| **GPIO32** | Output | PZEM-004T (A) | RX | Serial2 TX: Kirim perintah baca Blok A |
| **GPIO26** | Input | PZEM-004T (B) | TX | Serial1 RX: Terima data energi Blok B |
| **GPIO25** | Output | PZEM-004T (B) | RX | Serial1 TX: Kirim perintah baca Blok B |
| **GPIO21** | I/O | OLED + RTC | SDA | Jalur Data Bus I2C Bersama (Wire) |
| **GPIO22** | Output | OLED + RTC | SCL | Jalur Clock Bus I2C Bersama (Wire) |
| **GPIO27** | Output | Active Buzzer | VCC / Signal | Alarm Audio Peringatan & Beep Relai |
| **GPIO34** | Input Only | ZMPT101B | OUT (Analog) | ADC1_CH6 Pengukur Gelombang AC PLN |
| **GPIO2**  | Output | Internal LED | - | Indikator Status internal (Active LOW) |
| **VIN**    | Power In | Step-Down | +5V OUT | Catu daya utama mikrokontroler (5.0V) |
| **3V3**    | Power Out| PZEM / I2C | VCC 3.3V | Catu daya logika periferal 3.3V |
| **GND**    | Ground   | Semua Modul| GND | Common Ground seluruh sistem DC |

---

### 3.2 Diagram Koneksi Sensor Energi PZEM-004T (Dual UART)

PZEM-004T v3.0 menggunakan protokol Modbus-RTU over UART TTL logic level. Unit v3.0 dapat diberi daya logika 3.3V langsung dari ESP32 untuk sisi optocoupler serialnya, sementara bagian pengukuran AC terisolasi secara galvanis.

```
          ESP32 DEVKIT                               PZEM-004T (BLOK A)
     ┌──────────────────┐                           ┌──────────────────┐
     │         GPIO33   ├──────────────────────────►│ TX               │
     │         GPIO32   │◄──────────────────────────┤ RX               │
     │           3.3V   ├──────────────────────────►│ VCC              │
     │            GND   ├──────────────────────────►│ GND              │
     │                  │                           │                  │
     │                  │                           │ AC-L   AC-N   CT │
     │                  │                           └─┬───────┬──────┬─┘
     │                  │                             │       │      │
     │                  │                             │       │      └─► Klem CT pada
     │                  │                             │       │          Fasa Blok A
     │                  │                             │       │
     │                  │   PZEM-004T (BLOK B)        │       │
     │                  │  ┌──────────────────┐       │       │
     │         GPIO26   ├─►│ TX               │       │       │
     │         GPIO25   │◄─┤ RX               │       ▼       ▼
     │           3.3V   ├─►│ VCC              │      Jalur AC 220V
     │            GND   ├─►│ GND              │      PLN Beban
     └──────────────────┘  │                  │
                           │ AC-L   AC-N   CT │
                           └─┬───────┬──────┬─┘
                             │       │      │
                             ▼       ▼      └─► Klem CT pada
                            Jalur AC 220V       Fasa Blok B
```

> [!TIP]
> **Pemasangan CT (Current Transformer):**  
> Pastikan kabel yang dimasukkan ke dalam lubang klem CT **HANYA SATU KABEL FASA (LINE)**, bukan kabel ganda (Line dan Netral digabung). Jika kabel Line dan Netral dimasukkan bersamaan, medan magnet akan saling meniadakan dan pembacaan arus akan menunjukkan 0.00 A.

---

### 3.3 Diagram Sensor Tegangan PLN ZMPT101B

Sensor ZMPT101B mentransformasikan tegangan AC 220V menjadi sinyal analog sinusoidal kecil ber-offset 1.65V DC yang aman dibaca oleh pin ADC ESP32.

```
            SISI TEGANGAN TINGGI AC                  SISI LOGIKA DC (ESP32)
         ┌───────────────────────────┐            ┌───────────────────────────┐
         │ Jala-Jala PLN 220V        │            │ Modul ZMPT101B            │
         │ Live (Fasa) ─────────────►│ L      VCC ├────────► +5V Catu Daya    │
         │ Netral ──────────────────►│ N      GND ├────────► GND Bersama      │
         └───────────────────────────┘        OUT ├────────► GPIO34 (ESP32)   │
                                                  └───────────────────────────┘
```

* Pin **GPIO34** pada ESP32 adalah *Input Only* (tanpa pull-up internal) dan merupakan bagian dari blok **ADC1**, sehingga aman dibaca secara bersamaan saat WiFi aktif (tidak ada konflik dengan ADC2).

---

### 3.4 Diagram Bus I2C (OLED SSD1306 & RTC DS3231)

Layar OLED dan modul RTC DS3231 berbagi jalur komunikasi data serial sinkron I2C yang sama (*Two-Wire Interface*):

```
                       ESP32 DEVKIT
                   ┌──────────────────┐
                   │     3.3V   GND   │
                   └──────┬──────┬────┘
                          │      │
              ┌───────────┴──────┼───────────┐
              │                  │           │
     ┌────────▼────────┐         │  ┌────────▼────────┐
     │  OLED SSD1306   │         │  │   RTC DS3231    │
     │ (Alamat: 0x3C)  │         │  │ (Alamat: 0x68)  │
     │                 │         │  │                 │
     │ VCC  GND SDA SCL│         │  │ VCC  GND SDA SCL│
     └──┬────┬───┬───┬─┘         │  └──┬────┬───┬───┬─┘
        │    │   │   │           │     │    │   │   │
        └────┼───┼───┼───────────┘     │    │   │   │
             │   │   │                 │    │   │   │
  GPIO21 ────┴───┼───┴─────────────────┼────┴───┘   │ (SDA)
  GPIO22 ────────┴─────────────────────┴────────────┘ (SCL)
```

* Keduanya beroperasi pada level tegangan logika 3.3V.
* Firmware secara berkala memverifikasi register tanggal & waktu RTC. Jika terdeteksi tidak valid (misal baterai koin kendor), sistem menampilkan indikator `--:--:--` agar data sampah tidak mengaburkan tampilan pengguna.

---

### 3.5 Diagram Aktuator Relay & Kontaktor Lampu

Untuk menyalakan ratusan watt lampu LED buah naga di kebun, sinyal GPIO ESP32 memicu relay perantara yang kemudian menghidupkan koil kontaktor AC 220V:

```
 ESP32 GPIO         RELAY MODUL (5V)                 KONTAKTOR MAGNETIK             BEBAN LAMPU
┌─────────┐      ┌────────────────────┐            ┌────────────────────┐       ┌─────────────────┐
│ GPIO13  ├─────►│ IN 1   (Blok A)    │            │ Koil Kontaktor A1  │       │ Lampu Kebun     │
│ (Active │      │ NO 1 ──────────────┼───────────►│ (Tegangan AC 220V) │──────►│ Blok A (L1..Ln) │
│  LOW)   │      │ COM 1 ── [AC Fasa] │            │ Kontak Utama (1-2) │       └─────────────────┘
│         │      │                    │            └────────────────────┘
│ GPIO14  ├─────►│ IN 2   (Blok B)    │            ┌────────────────────┐       ┌─────────────────┐
│ (Active │      │ NO 2 ──────────────┼───────────►│ Koil Kontaktor B1  │       │ Lampu Kebun     │
│  LOW)   │      │ COM 2 ── [AC Fasa] │            │ (Tegangan AC 220V) │──────►│ Blok B (L1..Ln) │
└─────────┘      └────────────────────┘            │ Kontak Utama (3-4) │       └─────────────────┘
                                                   └────────────────────┘
```

> [!NOTE]
> Firmware menginisialisasi pin relay dengan urutan:  
> `digitalWrite(RELAY_PIN, HIGH);` lalu `pinMode(RELAY_PIN, OUTPUT);`  
> Prosedur ini krusial untuk mencegah terjadinya **glitch sementara (flicker)** saat mikrokontroler menyala atau reboot.

---

### 3.6 Diagram Alarm Buzzer

```
          ESP32                        ACTIVE BUZZER
     ┌──────────────┐                 ┌─────────────┐
     │    GPIO27    ├────────────────►│ VCC (+)     │
     │     GND      ├────────────────►│ GND (-)     │
     └──────────────┘                 └─────────────┘
```

* Pin buzzer dipindahkan ke **GPIO27** (sebelumnya di GPIO23) untuk menghindari interferensi sinyal frekuensi tinggi dan lonjakan floating saat proses bootloader.
* Kontrol buzzer dilakukan 100% *non-blocking* menggunakan timer `millis()`, sehingga sistem tidak pernah mengalami jeda eksekusi (*freeze/stall*).

---

## BAB IV: INSTALASI FISIK & KESELAMATAN KERJA (K3)

### 4.1 Standar Keamanan Tegangan Tinggi (AC 220V)

> [!CAUTION]
> **BAHAYA SENGATAN LISTRIK (220V AC):**  
> Jalur kontaktor, terminal MCB, input PZEM, dan transformator ZMPT101B mengalirkan arus listrik bertegangan tinggi yang mematikan.

1. **Wajib Memutus MCB Utama:** Jangan pernah menyambung, meraba, atau mengencangkan baut terminal saat MCB induk dalam keadaan ON.
2. **Uji Menggunakan Tespen / Multimeter:** Selalu uji kabel fasa dengan tespen sebelum memegang kabel untuk memastikan kawat benar-benar tidak bertegangan.
3. **Pemisahan Jalur Tegangan Rendah dan Tinggi:** Buat partisi fisik di dalam box panel antara kabel sinyal DC 3.3V/5V dengan kabel fasa beban AC 220V untuk mencegah induksi elektromagnetik dan bahaya korsleting.
4. **Gunakan Sepatu Berisolasi:** Selalu kenakan sepatu beralas karet kering saat merakit atau melakukan pengecekan panel di area kebun terbuka.

---

### 4.2 Tata Letak Box Panel & Terminal Blok

Rekomendasi penataan modul di dalam box panel outdoor IP65:

```
┌──────────────────────────────────────────────────────────────────┐
│                      BOX PANEL UTAMA (IP65)                      │
│                                                                  │
│  [ MCB Utama ]    [ MCB Beban A ]    [ MCB Beban B ]   [ TERMINAL]│
│  ┌───────────┐    ┌───────────┐      ┌───────────┐     │ INPUT   ││
│  │   16 A    │    │   10 A    │      │   10 A    │     │ AC 220V ││
│  └───────────┘    └───────────┘      └───────────┘     └─────────┘│
│                                                                  │
│  ┌─────────────────────────────┐   ┌───────────────────────────┐ │
│  │ Kontaktor Magnetik Blok A   │   │ Kontaktor Magnetik Blok B │ │
│  └─────────────────────────────┘   └───────────────────────────┘ │
│                                                                  │
│  ┌─────────────────────────────┐   ┌───────────────────────────┐ │
│  │ Modul PZEM-004T (Blok A)    │   │ Modul PZEM-004T (Blok B)  │ │
│  └─────────────────────────────┘   └───────────────────────────┘ │
│                                                                  │
│  ┌──────────────┐ ┌──────────────┐ ┌────────────┐ ┌────────────┐ │
│  │ ZMPT101B AC  │ │ Modul Relay  │ │ PS 12V 5A  │ │ Step-Down  │ │
│  └──────────────┘ └──────────────┘ └────────────┘ └────────────┘ │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │           PAPAN KONTROLER UTAMA (AKRILIK/PCB)               │ │
│  │    [ OLED Display ]        [ ESP32 ]          [ RTC DS3231 ] │ │
│  │    (Dilihat dr luar kaca)  [ Buzzer]                         │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
│  [ TERMINAL OUTPUT LAMPU A ]         [ TERMINAL OUTPUT LAMPU B ] │
└──────────────────────────────────────────────────────────────────┘
```

---

### 4.3 Pemasangan Current Transformer (CT) di Lapangan

1. Klem CT berupa inti magnetik terbelah (*split-core*). Buka pengunci klem.
2. Kalungkan klem melingkari kawat kabel **FASA** (kabel yang bertegangan/hitam/merah) yang keluar dari kontaktor menuju kebun.
3. Kunci kembali klem hingga berbunyi *klik*.
4. Pastikan steker kabel sinyal CT tertancap kokoh pada soket terminal 2-pin di modul PZEM-004T.
5. Ulangi prosedur yang sama untuk CT Blok B pada kawat fasa keluaran kontaktor B.

---

### 4.4 Proteksi Cuaca (Waterproofing & Suhu Tropis)

* **Gland Kabel (Cable Gland PG-7 / PG-9):** Seluruh kabel masuk dan keluar box panel wajib melewati lubang cable gland di bagian bawah box. Jangan membuat lubang di atas atau di samping atap box untuk mencegah rembesan air hujan.
* **Silica Gel:** Masukkan kantong silica gel industri berukuran 50-100 gram di sudut panel untuk menyerap kelembapan udara tropis malam hari.
* **Ventilasi Bertudung (Louver Filter):** Jika panel terpapar sinar matahari siang, gunakan kisi-kisi ventilasi berfilter busa agar sirkulasi udara power supply tetap dingin tanpa kemasukan serangga kebun atau percikan air.

---

## BAB V: PANDUAN PENGATURAN AWAL & KONEKTIVITAS (WIFIMANAGER)

### 5.1 Mode Hotspot Konfigurasi (SmartNaga-AP)

Jika perangkat SmartNaga baru pertama kali dinyalakan, atau jika router/modem WiFi kebun diganti SSID/password-nya, ESP32 tidak akan menemukan jaringan lama dalam 30 detik. Sistem secara otomatis mengaktifkan mode **Access Point (AP) On-The-Fly**:

* Layar OLED menampilkan teks:
  ```
  == SETUP WIFI ==

  1. Konek ke WiFi:
     SmartNaga-AP
  2. Pass: smartnaga123
  3. Buka 192.168.4.1
  ```

---

### 5.2 Langkah-Langkah Menghubungkan ke WiFi / Modem 4G

1. Siapkan Smartphone atau Laptop, buka menu **Pengaturan Wi-Fi**.
2. Cari nama jaringan (SSID): `SmartNaga-AP`.
3. Masukkan kata sandi default: `smartnaga123`.
4. Setelah terhubung, biasanya jendela login captive portal akan terbuka secara otomatis. Jika tidak muncul, buka peramban web (Google Chrome / Safari) dan ketik alamat IP: `192.168.4.1`.
5. Klik menu **Configure WiFi**.
6. Perangkat akan memindai seluruh sinyal WiFi di sekitarnya. Pilih nama WiFi kebun atau Hotspot Modem 4G yang diinginkan.
7. Ketikkan kata sandi WiFi kebun dengan teliti, lalu tekan tombol **Save**.
8. Perangkat ESP32 akan menyimpan kredensial ke memori Flash NVS non-volatile dan me-reboot dirinya sendiri.
9. Setelah reboot berhasil, alat otomatis tersambung ke internet dan OLED menampilkan alamat IP lokal beserta ping latency ke Google.

---

### 5.3 Indikator Status Jaringan & Ping Latency Test

Firmware SmartNaga tidak hanya mengecek apakah ESP32 terhubung ke router lokal, melainkan menjalankan paket **ICMP Ping ke DNS Google (8.8.8.8)** setiap 3 detik via task latar belakang ESP-IDF `esp_ping`:

| Baris Layar OLED | Arti Indikator | Kondisi Jaringan |
|---|---|---|
| `Wifi: online 45ms` | Internet Sangat Sehat | Terhubung ke router DAN internet lancar (latensi 45 ms) |
| `Wifi: No Internet` | Gangguan Provider | Terhubung ke router/WiFi, namun kuota habis atau modem no-service |
| `Wifi: offline` | Putus Sinyal Lokal | ESP32 terputus dari router WiFi kebun |
| `Wifi: PORTAL` | Mode Pengaturan | Sedang membuka hotspot konfigurasi `SmartNaga-AP` |

---

### 5.4 Prosedur Reset Kredensial WiFi Jarak Jauh

Jika ingin memindahkan alat ke kebun lain tanpa perlu membongkar box panel:
1. Buka aplikasi **SmartNaga Mobile App**.
2. Masuk ke menu **Pengaturan Perangkat** → pilih opsi **Reset Jaringan WiFi Perangkat**.
3. Aplikasi akan menulis nilai `true` ke path Firebase `/system/reset_wifi`.
4. Dalam interval 3 detik, ESP32 membaca instruksi tersebut, mengembalikan flag ke `false`, menghapus memori WiFi NVS, me-restart diri, dan langsung membuka kembali portal `SmartNaga-AP`.

---

## BAB VI: ANTARMUKA LOKAL & PENGOPERASIAN DISPLAY OLED

### 6.1 Struktur Tampilan Layar 7-Baris

Layar OLED 0.96 inci diatur dalam susunan 7 baris teks presisi (ukuran font 6x8 piksel) untuk menyajikan gambaran sistem yang lengkap dalam satu layar tanpa perlu memutar menu (*no paging*):

```
┌──────────────────────────────────────┐
│               19:45:12               │ ◄── Baris 1: Jam Real-Time RTC (DS3231)
│ Wifi: online 52ms                    │ ◄── Baris 2: Status Jaringan & Latensi
│ Source: PLN                          │ ◄── Baris 3: Sumber Daya Listrik (PLN / UPS)
│ Blok A: 840W                         │ ◄── Baris 4: Konsumsi Daya Blok A
│ Status: 12/12 Normal                 │ ◄── Baris 5: Diagnosa Status Lampu Blok A
│ Blok B: 0W                           │ ◄── Baris 6: Konsumsi Daya Blok B
│ Status: Standby                      │ ◄── Baris 7: Diagnosa Status Lampu Blok B
└──────────────────────────────────────┘
```

---

### 6.2 Arti Status & Notifikasi Visual

* **Baris 1 (Jam):** Menampilkan waktu 24 jam `HH:MM:SS`. Waktu ini sinkron otomatis dengan server NTP internet saat terkoneksi, dan dijaga akurasinya oleh osilator kristal temperatur DS3231 saat offline.
* **Baris 2 (WiFi):** Menampilkan kualitas tautan radio dan konektivitas cloud.
* **Baris 3 (Source):**
  * `Source: PLN` = Sumber listrik utama menyala stabil.
  * `Source: UPS` = Terdeteksi pemadaman PLN; sistem kontrol berjalan menggunakan suplai darurat UPS.
* **Baris 4 & 6 (Daya Blok):** Menampilkan pembacaan watt nyata hasil perhitungan `V x I x cos φ` dari PZEM-004T.
* **Baris 5 & 7 (Status Lampu):** Menampilkan kesimpulan algoritma pohon keputusan (lihat Bab VII).

---

### 6.3 Indikator Suara (Audio Alert / Buzzer)

Buzzer aktif menghasilkan pola nada khusus sebagai sinyal peringatan bagi petugas di kebun:

| Pola Suara Buzzer | Durasi | Peristiwa Pemicu | Makna & Tindakan |
|---|---|---|---|
| **1x Beep Singkat** | 200 ms | Alat Baru Menyala (Boot) | Sistem selesai inisialisasi & hardware siap kerja. |
| **1x Beep Konfirmasi** | 200 ms | Saklar Lampu Diklik (ON/OFF) | Relay merespon perintah aplikasi / jadwal. |
| **1x Beep Panjang** | 2000 ms (2 detik)| Listrik PLN Padam | Terjadi transisi dari PLN ke UPS. Petugas waspada. |

---

### 6.4 Mode Operasi Offline (Fail-Safe RTC DS3231)

Banyak perkebunan buah naga berada di perbukitan atau pesisir yang sering kehilangan sinyal seluler. SmartNaga didesain dengan prinsip **Fail-Safe Offline Autonomy**:

1. Jika koneksi internet terputus di tengah malam, sistem **TIDAK AKAN** mereset saklar lampu menjadi mati.
2. Siklus jadwal hidup/mati lampu diatur berdasarkan jam internal RTC DS3231.
3. Seluruh evaluasi pohon keputusan diagnosa beban tetap berjalan 100% lokal di CPU ESP32 dan terus diupdate di layar OLED setiap detik.
4. Ketika sinyal internet kembali pulih, akumulasi data energi (kWh) dan status terakhir seketika diunggah ke Firebase tanpa data hilang.

---

## BAB VII: LOGIKA DIAGNOSA CERDAS (DECISION TREE 10 STATUS)

### 7.1 Filosofi Firmware-First Diagnostic

Sebelum versi ini, evaluasi status lampu dilakukan pada aplikasi smartphone (Flutter). Namun hal itu memiliki kelemahan fatal: jika aplikasi ditutup atau petani sedang tidur, status anomali tidak terdeteksi secara kontinu dan layar OLED di kebun tidak dapat menampilkan status kerusakan.

Kini, **Firmware ESP32 bertindak sebagai Single Source of Truth (Pengevaluasi Tunggal)**. Parameter konfigurasi (`expected_power`, `lamp_count`, `tolerance_percent`) dikirim dari aplikasi dan disimpan di memori RAM ESP32, lalu dievaluasi setiap 1 detik.

---

### 7.2 Diagram Alir Evaluasi Status Beban

```
                            [ MULAI EVALUASI ]
                                     │
                        Sensor PZEM & Tegangan Siap?
                                     ├─── TIDAK ───► [ SENSOR ERROR ]
                                     │
                                    YA
                                     │
                              Relay Posisi ON?
                       ┌─────────────┴─────────────┐
                     TIDAK                         YA
                       │                           │
              Daya > 2W / Arus > 0.05A?     Tegangan < 50V?
               ├─── YA ──► [ RELAY BOCOR ]   ├─── YA ──► [ LISTRIK PADAM ]
               │                             │
             TIDAK                          Tegangan < 190V?
               │                             ├─── YA ──► [ TEG. RENDAH ]
               ▼                             │
        [ STANDBY NORMAL ]                  Tegangan > 245V?
                                             ├─── YA ──► [ TEG. TINGGI ]
                                             │
                                        Tegangan Normal
                                             │
                                  Daya < 30% Daya 1 Bohlam?
                                     ├─── YA ───► [ LAMPU PADAM TOTAL ]
                                     │
                             Daya Masuk Rentang Toleransi?
                                     ├─── YA ───► [ NORMAL AKTIF (N/N) ]
                                     │
                               Daya < Batas Bawah?
                                     ├─── YA ───► [ SEBAGIAN LAMPU RUSAK ]
                                     │
                                     └───► [ BEBAN LEBIH / OVERLOAD ]
```

---

### 7.3 Penjelasan 10 Status Diagnosa Lapangan

| No | Nama Kode (RTDB) | Label Layar OLED | Arti Kondisi Lapangan & Akar Masalah |
|:---:|---|---|---|
| 1 | `sensorError` | `Sensor Error` | Kabel komunikasi UART PZEM terlepas, baudrate salah, atau sensor rusak. |
| 2 | `plnOutage` | `Listrik Padam` | Relay memerintahkan ON namun tegangan < 50V (pemadaman gardu PLN). |
| 3 | `underVoltage` | `Teg. Rendah` | Tegangan jala-jala drop di bawah 190V. Berisiko merusak ballast/driver lampu LED. |
| 4 | `overVoltage` | `Teg. Tinggi` | Tegangan melonjak di atas 245V (bahaya over-voltage petir/netral lepas). |
| 5 | `normalStandby` | `Standby` | Relay posisi OFF, tidak ada arus yang mengalir (kondisi istirahat normal). |
| 6 | `normalActive` | `X/X Normal` | Lampu menyala sempurna; konsumsi daya berada dalam rentang toleransi konfigurasi. |
| 7 | `circuitBroken` | `Lampu Padam` | Relay ON & tegangan PLN ada, namun daya < 2W. MCB beban trip atau kabel fasa putus. |
| 8 | `partialLampFail`| `X/Y Normal` | Daya di bawah batas bawah. Beberapa bohlam LED putus/hilang di tiang kebun. |
| 9 | `overload` | `Beban Lebih` | Daya melebihi batas atas toleransi. Ada induksi bocor atau penambahan beban ilegal. |
| 10 | `relayLeakage` | `Relay Bocor` | Relay OFF namun terdeteksi daya > 2W. Kontak kontaktor/relay lengket atau bocor arus. |

> [!TIP]
> **Fitur Toggle Settling (4 Detik):**  
> Lampu LED dan trafo membutuhkan waktu beberapa ratus milidetik untuk stabilisasi arus inrush. Saat tombol saklar di-klik, sistem menahan evaluasi selama 4 detik (`TOGGLE_SETTLING_MS`) agar tidak memicu alarm palsu sesaat.

---

### 7.4 Mekanisme Estimasi Jumlah Lampu Putus

Jika status terdeteksi `partialLampFail`, firmware menghitung estimasi jumlah lampu yang putus dengan rumus:

$$\text{Daya Per Lampu (W)} = \frac{\text{Expected Power}}{\text{Jumlah Lampu}}$$

$$\text{Estimasi Lampu Mati} = \text{round}\left(\frac{\text{Expected Power} - \text{Daya Terukur}}{\text{Daya Per Lampu}}\right)$$

$$\text{Lampu Normal Menyala} = \text{Jumlah Lampu} - \text{Estimasi Lampu Mati}$$

*Contoh:* Kebun Blok A dikonfigurasi 10 buah lampu LED 15W (Total: 150W). Jika terbaca daya aktual 120W, sistem menghitung bahwa 2 buah lampu mati, dan layar OLED secara jelas menuliskan: `Status: 8/10 Normal`.

---

## BAB VIII: INTEGRASI CLOUD & APLIKASI MOBILE

### 8.1 Sinkronisasi Firebase Realtime Database

ESP32 secara terus-menerus bertukar data dua arah dengan Firebase Realtime Database melalui koneksi HTTPS/SSL terotentikasi.

```
Firebase RTDB Root
├── /energy/
│   ├── block_a/        ─── [Tegangan, Arus, Watt, kWh Blok A] (update tiap 2.5s)
│   ├── block_b/        ─── [Tegangan, Arus, Watt, kWh Blok B] (update tiap 2.5s)
│   ├── z_mpt/          ─── [Status PLN true/false, Tegangan Kalibrasi RMS]
│   └── pln_status/     ─── [available, source: "PLN"/"UPS"]
├── /lamp/
│   ├── control/        ─── [a: bool, b: bool] (DITULIS APLIKASI, DIBACA ESP32)
│   ├── status/         ─── [state, timestamp, t_esp_exec] (KONFIRMASI EKSEKUSI)
│   └── diag/           ─── [status, brokenLamps, isAlert, timestamp] (DIAGNOSA ESP32)
├── /config/
│   ├── block_a/        ─── [expected_power, lamp_count, tolerance_percent]
│   └── block_b/        ─── [expected_power, lamp_count, tolerance_percent]
├── /device/
│   ├── heartbeat/      ─── [epoch timestamp per detik]
│   └── ping/           ─── [online, latency_ms, ssid, ip, mac]
└── /system/
    └── reset_wifi/     ─── [bool trigger reset portal]
```

---

### 8.2 Struktur Payload Telemetri & Kontrol

Contoh data JSON telemetri riil yang dikirim oleh ESP32 ke `/energy/block_a`:
```json
{
  "voltage": 224.6,
  "current": 4.125,
  "power": 925.8,
  "energy": 14.852,
  "valid": true,
  "timestamp": 1731139200000
}
```

Contoh data diagnosa cerdas di `/lamp/diag/a`:
```json
{
  "status": "partialLampFail",
  "brokenLamps": 2,
  "isAlert": true,
  "settling": false,
  "timestamp": 1731139200
}
```

---

### 8.3 Fitur Kontrol Manual & Penjadwalan Lampu

Pada aplikasi mobile **SmartNaga App**:
1. **Saklar Manual Real-Time:** Menekan tombol saklar pada layar akan mengubah nilai `/lamp/control/a` menjadi `true` atau `false`. Firmware ESP32 mendeteksi perubahan tersebut dalam waktu <100 ms dan memicu relay.
2. **Penjadwalan Otomatis (Scheduler):** Pengguna dapat menentukan jadwal rutin harian (contoh: Nyala pukul 18.00 WIB, Padam pukul 23.00 WIB). Scheduler akan mengeksekusi kontrol secara nirkabel.
3. **Penyelarasan Awal Saat Booting:** Jika ESP32 mengalami mati listrik lalu hidup kembali, firmware pertama kali membaca `/lamp/control/{a,b}` di cloud untuk memulihkan status lampu sesuai kondisi terakhir yang tercatat di aplikasi.

---

### 8.4 Konfigurasi Parameter Beban (Expected Power & Toleransi)

Agar deteksi anomali berjalan presisi, teknisi wajib memasukkan konfigurasi beban kebun melalui menu **Konfigurasi Anomali** di aplikasi mobile:

1. **Daya Harapan (Expected Power):** Total daya nominal seluruh lampu yang terpasang pada satu blok (misal: 100 lampu x 9 Watt = 900 Watt).
2. **Jumlah Lampu (Lamp Count):** Total fisik bohlam yang terpasang pada blok tersebut (misal: 100).
3. **Persentase Toleransi (Tolerance %):** Ambang batas fluktuasi normal tegangan pedesaan (disarankan: 15% - 20%).

Data ini disimpan ke `/config/block_a` dan `/config/block_b`. Firmware membaca konfigurasi ini secara berkala dan menerapkannya langsung pada pohon keputusan.

---

## BAB IX: KALIBRASI SENSOR & PENGUJIAN AKURASI

### 9.1 Kalibrasi Sensor Tegangan ZMPT101B

Sensor ZMPT101B memerlukan penyesuaian trimmer potensiometer multi-turn biru agar sinyal sinus tidak terpotong (*clipping*) pada rentang ADC 0-4095:

1. Siapkan obeng minus presisi kecil.
2. Hubungkan terminal AC ZMPT ke stopkontak PLN 220V.
3. Buka serial monitor PlatformIO pada baudrate `115200` dan amati baris log:
   `[ZMPT] RMS: 356.7 (238.0V) P2P: 1024 Base: 1845.0`
4. Putar baut potensiometer pelan-pelan sampai nilai **Base (DC Offset)** berada di sekitar **1700 - 1900** (mendekati nilai tengah ADC 12-bit ESP32).
5. Pastikan nilai **P2P (Peak-to-Peak)** berada di kisaran **800 - 1600**. Nilai tidak boleh mendekati 4095 agar ADC tidak jenuh.
6. **Faktor Kalibrasi K:** Nilai tegangan aktual dihitung dengan rumus:
   $$V_{\text{real}} = \text{RMS Raw} \times K$$
   Pada firmware terkonfigurasi nilai teruji lapangan: $K = 0.667$.

---

### 9.2 Kalibrasi Sensor PZEM-004T v3.0

PZEM-004T v3.0 telah terkalibrasi secara digital dari pabrik (*factory calibrated*) menggunakan chip metering SD3004 berstandar kelas 1.0.

* Jika nilai kWh ingin di-reset kembali ke angka 0 saat musim panen baru dimulai, dapat dikirimkan instruksi reset energy Modbus melalui perintah serial atau tombol reset di aplikasi mobile.

---

### 9.3 Validasi Akurasi Waktu RTC DS3231 via NTP

Modul RTC DS3231 memiliki osilator internal yang terkompensasi suhu.
1. Saat perangkat terhubung ke internet dan berhasil melakukan *ping Google OK*, fungsi `syncRTC()` memanggil server NTP Indonesia (`pool.ntp.org` / timezone GMT+7).
2. Waktu RTC disinkronkan hingga satuan detik.
3. Baterai koin **CR2032** pada modul RTC mampu mempertahankan akurasi jam hingga **3 hingga 5 tahun** meskipun alat disimpan dalam gudang tanpa daya.

---

## BAB X: PANDUAN PEMELIHARAAN & PEMECAHAN MASALAH (TROUBLESHOOTING)

### 10.1 Tabel Matriks Masalah & Solusi Cepat

| Gejala Gangguan | Kemungkinan Penyebab | Tindakan Perbaikan |
|---|---|---|
| **Layar OLED gelap gulita (Blank)** | Kabel jumper I2C kendor atau alamat I2C salah | Periksa kabel SDA (GPIO21) dan SCL (GPIO22). Pastikan pin VCC OLED terhubung ke 3.3V dan GND terpasang rapat. |
| **Layar OLED menampilkan `--:--:--`** | Baterai RTC habis atau modul DS3231 kendor | Periksa modul RTC di bus I2C. Ganti baterai koin CR2032 dengan yang baru. Hubungkan alat ke internet agar waktu tersinkron via NTP. |
| **Status `Sensor Error` pada Blok A/B** | Kabel serial UART terbalik atau PZEM tidak bertegangan | Pastikan jalur UART: TX PZEM ke RX ESP32 dan sebaliknya. Pastikan terminal AC PZEM tersambung ke 220V (PZEM butuh AC untuk menyalakan optoisolator). |
| **Daya terbaca 0 Watt padahal lampu menyala** | Klem CT menjepit kabel ganda (Line + Neutral) | Lepaskan klem CT. Kalungkan klem **HANYA** pada kawat fasa tunggal. Periksa steker jack 3.5mm / terminal CT ke modul PZEM. |
| **Status `Listrik Padam` padahal PLN ada** | Potensiometer ZMPT belum diputar atau sensitivitas drop | Putar potensiometer ZMPT101B searah jarum jam untuk menaikkan nilai amplitudo RMS hingga >150. Periksa kabel AC input ZMPT. |
| **Buzzer berbunyi panjang 2 detik terus-menerus** | Listrik PLN sedang padam atau terjadi kontak goyang ZMPT | Periksa MCB PLN kebun. Jika PLN hidup namun alarm berbunyi, periksa kekencangan baut terminal kabel ZMPT. |
| **Status `Lampu Padam` (Circuit Broken)** | MCB beban trip atau ada kabel lampu yang putus digigit hama | Periksa tuas MCB beban 10A di dalam panel. Periksa sambungan kabel fasa keluar dari kontaktor menuju tiang kebun. |
| **Status `Lampu Rusak` (Partial Fail)** | Beberapa bohlam LED putus/mati di tiang kebun | Lihat angka pada layar OLED (misal `8/10 Normal` berarti 2 lampu mati). Sisir deretan tiang kebun dan ganti lampu yang padam. |
| **Status `Relay Bocor` saat lampu OFF** | Kontak kontaktor lengket (*welded*) atau ada induksi | Ketuk perlahan bodi kontaktor jika kontak mekanis lengket. Jika masih terjadi kebocoran arus >0.05A, ganti modul relay atau kontaktor. |
| **WiFi bolak-balik diskonek / loop freeze** | Sinyal modem lemah atau power supply drop saat transmit | Pastikan antena modem 4G terarah dengan baik. Pastikan tegangan output stepdown LM2596 stabil di 5.0V saat ESP32 transmit WiFi. |

---

### 10.2 Pemeliharaan Preventif Bulanan & Musiman

* **Pemeriksaan Baut Terminal (Setiap 3 Bulan):** Getaran dan perubahan suhu dapat melonggarkan baut terminal blok. Kencangkan seluruh baut kabel daya AC dan terminal DC menggunakan obeng berisolasi.
* **Pembersihan Debu & Penggantian Silika Gel (Setiap 6 Bulan):** Bersihkan debu halus perkebunan yang menempel pada PCB menggunakan kuas kering berbulu halus. Ganti kantong silika gel jika butirannya telah berubah warna menjadi jenuh/merah muda.
* **Pengecekan Baterai Cadangan UPS (Setiap 6 Bulan):** Matikan MCB induk PLN untuk mensimulasikan pemadaman. Pastikan ESP32, OLED, dan modem tetap menyala normal setidaknya selama 30-60 menit menggunakan daya baterai cadangan.

---

### 10.3 Prosedur Pembaruan Firmware (USB PlatformIO & OTA WiFi)

#### Metode A: Upload Melalui Kabel USB (PlatformIO)
1. Sambungkan kabel data Micro-USB dari port USB laptop ke port ESP32.
2. Buka terminal proyek di VS Code / PlatformIO.
3. Jalankan perintah kompilasi dan upload:
   ```bash
   # Build dan upload firmware production
   pio run -t upload -e esp32dev

   # Buka serial monitor
   pio device monitor -e esp32dev
   ```

#### Metode B: Update Nirkabel Jarak Jauh (ArduinoOTA over WiFi)
Firmware SmartNaga telah dilengkapi pustaka `ArduinoOTA` bawaan:
1. Pastikan laptop berada dalam satu jaringan WiFi yang sama dengan ESP32 kebun.
2. Port jaringan akan terdeteksi di PlatformIO sebagai `smartnaga-esp32.local` beserta alamat IP-nya.
3. Lakukan upload nirkabel tanpa perlu membuka box panel di tiang kebun.

---

```
                      AKHIR DOKUMEN MANUAL BOOK
   SmartNaga IoT System — Solusi Cerdas Efisiensi Budidaya Buah Naga
```

