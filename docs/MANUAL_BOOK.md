# BUKU PANDUAN PENGGUNA & TEKNISI (MANUAL BOOK)
## Energa — Sistem Kontrol & Monitoring Pencahayaan Buah Naga Berbasis IoT

**Versi Dokumen:** 1.0 (Draft)  
**Tanggal Rilis:** 2026-11-09  
**Platform:** ESP32 DevKit V1 + Firebase RTDB + Energa Mobile App (Flutter)

---

## DAFTAR ISI

- [BAB 1: INFORMASI UMUM & SPESIFIKASI SISTEM](#bab-1-informasi-umum--spesifikasi-sistem)
- [BAB 2: PETUNJUK KESELAMATAN & K3 LISTRIK](#bab-2-petunjuk-keselamatan--k3-listrik)
- [BAB 3: TATA LETAK & PENGENALAN KOMPONEN PANEL](#bab-3-tata-letak--pengenalan-komponen-panel)
- [BAB 4: PANDUAN INSTALASI & PENGKABELAN (WIRING)](#bab-4-panduan-instalasi--pengkabelan-wiring)
- [BAB 5: PROSEDUR PENGOPERASIAN](#bab-5-prosedur-pengoperasian)
- [BAB 6: SISTEM DETEKSI ANOMALI & ALARM](#bab-6-sistem-deteksi-anomali--alarm)
- [BAB 7: PEMELIHARAAN BERKALA (MAINTENANCE)](#bab-7-pemeliharaan-berkala-maintenance)
- [BAB 8: PANDUAN PEMECAHAN MASALAH (TROUBLESHOOTING)](#bab-8-panduan-pemecahan-masalah-troubleshooting)

---

## BAB 1: INFORMASI UMUM & SPESIFIKASI SISTEM

### 1.1 Deskripsi Produk
**Energa** adalah sistem kontrol otomasi dan pemantauan pencahayaan kebun buah naga berbasis *Internet of Things* (IoT). Sistem ini dirancang untuk mengatur siklus penyinaran lampu secara presisi guna merangsang pembungaan di luar musim (*off-season flowering*), sekaligus memantau parameter kualitas daya listrik (tegangan, arus, daya, energi akumulasi) serta kondisi kesehatan instalasi lampu di lapangan secara real-time.

Sistem terdiri dari 3 pilar utama:
1. **Hardware Panel Kontrol (Edge Layer):** Box panel listrik terintegrasi dengan mikrokontroler ESP32, dual sensor energi (PZEM-004T), sensor deteksi suplai jala-jala (ZMPT101B), modul jam presisi (RTC DS3231), layar status OLED, serta kontaktor daya magnetik modular dengan peredam spike induktif (RC snubber).
2. **Cloud Backend:** Firebase Realtime Database (RTDB) untuk sinkronisasi perintah kontrol dan telemetri real-time dengan latensi rendah, serta Firebase Firestore untuk pengarsipan histori konsumsi energi jangka panjang.
3. **Energa Mobile App (Application Layer):** Aplikasi mobile berbasis Flutter untuk pemantauan parameter listrik, konfigurasi ambang batas toleransi beban, penjadwalan otomatis, dan kontrol manual.

---

### 1.2 Prinsip Arsitektur Dual-Bus (Pemisahan Jalur Daya & Kontrol)
Untuk menjamin keandalan sistem pemantauan di area perkebunan yang rawan pemadaman listrik, Energa menerapkan prinsip **Pemisahan Jalur Daya (Dual-Bus Architecture)**:

```
┌─────────────────────────────────────────────────────────────────┐
│              JALUR DAYA BEBAN LAMPU (PLN Saja)                  │
│                                                                 │
│   PLN 220V ──► [ZMPT101B] ──┬── Kontaktor A ──► Lampu Blok A    │
│                 (Deteksi)   └── Kontaktor B ──► Lampu Blok B    │
│                                                                 │
│   * Tanpa backup UPS: Lampu otomatis padam saat PLN padam.       │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│              JALUR SISTEM KONTROL (Ditopang UPS)                │
│                                                                 │
│   UPS 220V ──► SMPS 12V ──► Step-Down 5V ──► ESP32 + Sensor     │
│                                              + OLED + Modem 4G  │
│                                                                 │
│   * Tetap menyala saat PLN padam: ESP32 dapat membunyikan       │
│     alarm lokal dan mengirim notifikasi pemadaman ke cloud.     │
└─────────────────────────────────────────────────────────────────┘
```

1. **Jalur Daya Beban Lampu (High-Power AC Bus):**
   - Menyuplai lampu kebun (Blok A dan Blok B) langsung dari jaringan PLN 220V AC.
   - Jalur ini **tidak** di-backup oleh UPS karena konsumsi daya lampu kebun yang besar.
   - Diputus dan dihubungkan oleh Kontaktor Magnetik Modular TOMZN TOCT1-25.
2. **Jalur Kontrol & Telemetri (Low-Power Isolated Bus):**
   - Menyuplai unit kontrol internal: Switching Power Supply (SMPS), mikrokontroler ESP32, sensor ZMPT101B, modul PZEM-004T, RTC, OLED, serta modem 4G LTE/WiFi.
   - Diberi daya cadangan melalui **UPS AC 220V**.
   - Ketika listrik PLN padam, sistem kontrol tetap beroperasi normal untuk mendeteksi pemadaman, membunyikan alarm buzzer lokal, dan mengirim notifikasi darurat pemadaman ke smartphone pengguna.

---

### 1.3 Spesifikasi Teknis Perangkat

| Parameter | Spesifikasi Teknis | Keterangan / Referensi Aktual |
|---|---|---|
| **Catu Daya Masukan Utama (PLN)** | 220V AC ± 15%, 50 Hz | Input jalur fasa & netral utama ke terminal panel |
| **Catu Daya Cadangan Kontrol** | UPS 220V AC (Line-Interactive / Offline) | Khusus mencatu sistem logika, sensor, dan komunikasi |
| **Catu Daya Internal DC** | Switching Power Supply (SMPS) 12V DC + Buck Converter 5V DC | Menyuplai 5V ke board utama dan 3.3V via regulator ESP32 |
| **Mikrokontroler Utama** | ESP32 DevKit V1 (38-Pin) | Dual-core Tensilica Xtensa 32-bit LX6 @ 240 MHz |
| **Kapasitas Kanal Output** | 2 Kanal Beban Terpisah (Blok A & Blok B) | Masing-masing dikendalikan kontaktor mandiri |
| **Aktuator Beban Utama** | 2x Kontaktor Modular TOMZN TOCT1-25 | Rating nominal 25A (AC-7a / Beban resistif-kapasitif) |
| **Proteksi Kontak Daya** | 2x Modul RC Snubber (Capacitor-Resistor) | Terpasang paralel pada kontak beban untuk meredam arc/spike |
| **Sensor Energi** | 2x PZEM-004T v3.0 (CT Eksternal 100A) | Port UART: Blok A (GPIO 33/32), Blok B (GPIO 26/25) |
| **Rentang Pengukuran Energi** | V: 80–260V \| I: 0–100A \| P: 0–23 kW \| E: 0–9999 kWh | Akurasi pengukuran ± 1.0% |
| **Sensor Status Sumber Daya** | Modul ZMPT101B AC Voltage Sensor | Terhubung ke ADC1_CH6 (GPIO 34), ambang batas RMS: 150 |
| **Jam Real-Time (RTC)** | DS3231 I2C Real-Time Clock | Akurasi TCXO tinggi + Baterai kancing CR2032 (3V) |
| **Antarmuka Visual Lokal** | Layar OLED Monokrom 0.96" SSD1306 | Resolusi 128x64 pixel, antarmuka I2C (Address 0x3C) |
| **Indikator Audio** | Active Buzzer 5V (GPIO 27) | Alarm nada putus-putus saat PLN padam / anomali daya |
| **Konektivitas Nirkabel** | WiFi 802.11 b/g/n (2.4 GHz) | Mendukung On-The-Fly WiFiManager (SSID AP: `Energa-AP`, Open / Tanpa Password) |
| **Protokol Komunikasi Cloud** | HTTPS / WebSocket ke Firebase RTDB | Update telemetri per 2.5 detik; ping internet per 3 detik |
| **Tingkat Perlindungan Box** | Enclosure Panel Metal IP54/IP65 (Outdoor Wall/Pole Mount) | Tahan debu dan percikan air untuk lingkungan kebun |



---

## BAB 2: PETUNJUK KESELAMATAN & K3 LISTRIK

### 2.1 Simbol dan Tingkat Peringatan
Perhatikan simbol keselamatan berikut sebelum melakukan instalasi, pengoperasian, maupun pemeliharaan panel Energa:

> **BAHAYA (DANGER):** Menunjukkan adanya tegangan tinggi mematikan (220V AC). Kontak langsung tanpa prosedur isolasi dapat menyebabkan cedera parah hingga kematian.
>
> **PERINGATAN (WARNING):** Menunjukkan kondisi yang berpotensi merusak perangkat, memicu percikan api, atau menyebabkan kegagalan sistem otomasi.
>
> **PERHATIAN (CAUTION):** Menunjukkan instruksi penting terkait prosedur pengoperasian yang benar dan pencegahan degradasi komponen.

---

### 2.2 Peringatan Bahaya Sumber Ganda (Dual-Power Hazard)

> **PERINGATAN KRUSIAL: TEGANGAN TETAP AKTIF SAAT PLN MATI / DICABUT!**  
> Panel ini menggunakan arsitektur catu daya ganda (**PLN + UPS Backup**). Mematikan MCB PLN atau mencabut steker utama PLN **TIDAK** memadamkan seluruh tegangan di dalam box panel, karena inverter UPS akan tetap menyuplai tegangan 220V AC ke jalur kontrol (stop kontak Schneider, SMPS, dan terminal terkait).

**Prosedur Isolasi Daya Total (Zero-Energy State):**
Sebelum menyentuh terminal kabel terbuka atau melakukan penggantian komponen:
1. Putuskan sumber listrik PLN utama melalui MCB pemutus instalasi kebun.
2. Matikan unit UPS dengan menekan tombol daya (Power Switch) pada UPS hingga mati total (*bip* panjang berhenti dan lampu indikator padam).
3. Cabut steker hitam output UPS dari stop kontak modular DIN rail Schneider.
4. Gunakan tespen atau multimeter AC untuk memastikan tidak ada lagi tegangan sisa pada terminal sebelum memulai pekerjaan.

---

### 2.3 Pemisahan Zona Tegangan Rendah vs Tegangan Tinggi
Untuk mencegah sengatan listrik dan interferensi induksi (*noise* elektromagnetik) pada mikrokontroler, panel dibagi menjadi dua zona fisik:

```
┌─────────────────────────────────────────────────────────────────┐
│ [ZONA TEGANGAN RENDAH / DC SELV]                                │
│ Papan PCB Atas: ESP32 (3.3V), OLED, RTC, Step-Down (5V), ZMPT   │
│ ⚠️ Aman disentuh saat uji display, namun hindari tumpahan air/  │
│    logam konduktif yang dapat menyebabkan korsleting pin GPIO.   │
├─────────────────────────────────────────────────────────────────┤
│ [ZONA TEGANGAN TINGGI / AC 220V]                                │
│ Rel DIN Tengah & Bawah: Kontaktor TOMZN, Terminal Block Gray,   │
│ Terminal Input SMPS, Stop Kontak Schneider, & PZEM AC Terminal  │
│ ⛔ DILARANG KERAS DISENTUH KETIKA SUMBER DAYA (PLN/UPS) AKTIF!  │
└─────────────────────────────────────────────────────────────────┘
```

1. **Aturan Jalur Kabel (Routing):**
   - Kabel AC 220V (Fasa/Netral beban lampu dan daya kontaktor) harus ditata rapi pada rel DIN bawah dan **tidak boleh** melintang tanpa pelindung di atas area board mikrokontroler ESP32.
   - Modul RC snubber (kapasitor merah) terhubung langsung pada terminal kontaktor 220V AC dan menyimpan muatan singkat; hindari menyentuh kakinya sesaat setelah daya diputus.

---

### 2.4 Sistem Pembumian (Grounding / Protective Earth)
1. **Kabel Grounding Rel DIN:**
   - Di sisi kiri rel DIN aluminium (dekat stop kontak Schneider), terpasang mur dan baut pembumian dengan kabel kuning-garis-hijau (Protective Earth/PE).
   - Kabel ini wajib terhubung ke batang tembaga *grounding rod* yang tertancap ke tanah kebun.
2. **Ketentuan Tahanan Tanah:**
   - Nilai resistansi pembumian wajib memenuhi standar keamanan kelistrikan (maksimal ≤ 5 Ohm).
   - Pembumian yang buruk meningkatkan risiko rusaknya sensor PZEM/ESP32 akibat lonjakan tegangan transien (induksi petir di kebun terbuka) serta berisiko menyetrum saat bodi panel tersentuh.

---

### 2.5 Standar K3 Lingkungan Perkebunan (Outdoor Agri-Environment)
Instalasi di kebun buah naga memiliki tantangan lingkungan berupa kelembaban tinggi di malam hari, embun, debu tanah, serta serangga/tikus:

1. **Kondisi Pintu Panel:**
   - Pintu box panel harus **selalu terkunci rapat** selama operasional normal. Gasket karet pintu harus dalam kondisi elastis dan tidak getas agar standar perlindungan (IP54/IP65) tetap terjaga.
2. **Kondisi Cuaca & Kelembaban:**
   - **DILARANG** membuka pintu panel saat hujan lebat atau saat sistem irigasi/sprinkler kebun sedang aktif.
   - Pastikan tangan dan perkakas kerja dalam kondisi kering sempurna sebelum menyentuh panel.
3. **Alat Pelindung Diri (APD) Wajib untuk Teknisi:**
   - Sepatu keselamatan berisolasi listrik (*safety shoes* bersol karet).
   - Obeng dan tang berisolasi standar VDE / IEC 60900 (rating 1000V).
   - Multimeter digital dengan kategori proteksi minimal CAT III 600V.

---

### 2.6 Prosedur Tindakan Darurat (Emergency Response)
Jika terdeteksi salah satu kondisi darurat berikut:
- Muncul percikan api (*arcing*) atau kepulan asap di dalam box panel.
- Terdengar suara dengung keras abnormal dari kontaktor atau transformator.
- Tercium bau isolasi kabel hangus/terbakar.

**Langkah Penanganan:**
1. **JANGAN MENYENTUH BODI LOGAM PANEL DENGAN TANGAN TELANJANG.**
2. Segera putuskan sumber listrik utama kebun dari MCB induk di luar box panel.
3. Jika aman dijangkau, matikan sakelar UPS atau cabut kabel catu daya dari stop kontak luar.
4. Gunakan Alat Pemadam Api Ringan (APAR) bertipe **Kelas C / Clean Agent CO2** (khusus kebakaran listrik). **Dilarang keras menyiram air ke dalam box panel.**


---

## BAB 3: TATA LETAK & PENGENALAN KOMPONEN PANEL

### 3.1 Visualisasi Tata Letak Box Panel (Hardware Layout)

Berikut adalah panduan tata letak komponen di dalam box panel kontrol **Energa**. Foto di bawah ini telah dipetakan dengan nomor callout (1 s/d 14) untuk memudahkan identifikasi perangkat fisik di lapangan:

```text
┌──────────────────────────────────────────────────────────────────────────────────┐
│                               BOX PANEL ENERGA                                   │
│                                                                                  │
│   ┌──────────────┐          ┌───────────────────────────────────────────────┐    │
│   │     [1]      │          │  [2] Step-Down   [3] RTC DS3231  [4] OLED     │    │
│   │ Power Supply │          │  ┌───────────┐   ┌────────────┐  ┌──────────┐ │    │
│   │  SMPS 12V    │          │  └───────────┘   └────────────┘  └──────────┘ │    │
│   │              │          │  [6] ZMPT101B    [5] ESP32       [7] Relay 2Ch│[8] │
│   │  (Sarang     │          │  ┌───────────┐   ┌────────────┐  ┌──────────┐ │USB │
│   │   Tawon)     │          │  └───────────┘   └────────────┘  └──────────┘ │    │
│   └──────────────┘          └───────────────────────────────────────────────┘    │
│                                      [9A] PZEM Blok A    [9B] PZEM Blok B        │
│                                      ┌──────────────┐    ┌──────────────┐        │
│                                      └──────────────┘    └──────────────┘        │
│   ────────────────────────────────────────────────────────────────────────────   │
│    [10] Ground PE   [11] Stop Kontak    [12] Terminal Strip   [13] Kontaktor     │
│    (Baut Kuning)    Schneider DIN       Block (Gray DIN)      TOMZN + Snubber    │
│   ────────────────────────────────────────────────────────────────────────────   │
│                                                                                  │
│   ┌──────────────────────────────────────────────────────────────────────────┐   │
│   │                                  [14]                                    │   │
│   │              Unit Cadangan Daya (UPS - Backup Baterai)                   │   │
│   └──────────────────────────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────────────────────────┘
```

> **Catatan Penyisipan Foto:**  
> Letakkan file foto beranotasi garis & nomor di direktori `images/panel-layout-annotated.jpg` untuk dilampirkan pada versi cetak atau PDF:
> ```markdown
> ![Tata Letak Komponen Box Panel Energa](images/panel-layout-annotated.jpg)
> ```

---

### 3.2 Tabel Identifikasi & Spesifikasi Komponen

| No | Nama Komponen | Model / Tipe Fisik | Tegangan Kerja | Fungsi Utama di Sistem Energa |
|:--:|---|---|:--:|---|
| **1** | **Power Supply Jaring (SMPS)** | Switching PSU Metal Casing | Input: 220V AC<br>Output: 12V DC | Mengubah daya AC (dari output UPS) menjadi tegangan DC 12V stabil untuk mencatu papan kontrol utama. |
| **2** | **Modul Step-Down Regulator** | Buck Converter DC-DC (LM2596) | Input: 12V DC<br>Output: 5V DC | Menurunkan tegangan 12V DC dari SMPS menjadi 5.0V DC teregulasi untuk menyuplai ESP32 VIN, relay, dan sensor ZMPT101B. |
| **3** | **Modul Jam Presisi (RTC)** | DS3231 I2C + Baterai CR2032 | 3.3V DC (I2C) | Menjaga pewaktuan dan jadwal penyinaran lampu kebun tetap akurat detik-per-detik secara mandiri (*offline*), bahkan saat internet mati. Baterai kancing menjaga memori jam saat panel mati total. |
| **4** | **Layar Tampilan Lokal (OLED)** | OLED 0.96" SSD1306 (128x64) | 3.3V DC (I2C) | Menampilkan informasi real-time di lapangan: Jam RTC, status WiFi & latensi ping, status sumber daya (PLN/UPS), serta daya aktif (Watt) dan status kesehatan lampu Blok A & B. |
| **5** | **Mikrokontroler Utama (MCU)** | ESP32 DevKit V1 (38-Pin) | 5V (VIN) / 3.3V Logic | Otak pengendali (*Edge Controller*). Memproses pembacaan sensor, menjalankan algoritma diagnosa lampu (*decision tree*), mengontrol kontaktor, dan mengirim telemetri ke Firebase. |
| **6** | **Modul Sensor Tegangan AC** | ZMPT101B Voltage Transformer | VCC: 5V DC<br>Sinyal: ADC 3.3V | Mendeteksi level tegangan jala-jala PLN secara aman melalui trafo isolasi mikro (GPIO 34). Digunakan sistem untuk membedakan suplai PLN vs cadangan UPS secara instan (<100ms). |
| **7** | **Modul Relay Pengendali** | Relay 2-Channel 5V (Optocoupler) | Koil: 5V DC<br>Trigger: Active LOW | Bertindak sebagai sakelar perantara. Menerima sinyal GPIO rendah (LOW) dari ESP32 (GPIO 13 & 14) untuk menyalakan koil elektromagnetik Kontaktor Daya 220V AC. |
| **8** | **Port USB / Port Ekspansi** | USB Type-A Female | 5V DC | Port ekspansi daya DC untuk menyuplai modem 4G LTE eksternal atau kebutuhan pengujian perangkat di lapangan. |
| **9** | **Sensor Energi Listrik (2x)** | PZEM-004T v3.0 (CT Eksternal) | Sinyal: 3.3V UART<br>AC: 80–260V | Mengukur tegangan (V), arus (A), daya (W), dan energi (kWh) kumulatif secara presisi tinggi.<br>• **9A (Kiri):** Monitoring Beban Lampu Blok A.<br>• **9B (Kanan):** Monitoring Beban Lampu Blok B. |
| **10** | **Baut & Kabel Pembumian (PE)** | Terminal Baut Grounding Rel DIN | 0V (Earth) | Titik terminasi kabel *Protective Earth* (kuning-hijau). Menghubungkan rel DIN dan bodi logam panel ke grounding tanah kebun untuk mengalirkan arus bocor dan proteksi induksi petir. |
| **11** | **Stop Kontak Modular DIN Rail** | Schneider Electric Modular Socket | 220V AC (16A) | Colokan standar modular yang terpasang di rel DIN. Menjadi penghubung daya AC dari output UPS menuju steker kabel Power Supply (SMPS). |
| **12** | **Deretan Terminal Block** | DIN Rail Terminal Strip (Gray) | 220V AC / 500V Max | Titik distribusi kabel AC fasa, netral, dan percabangan kabel instalasi kebun agar rapi, kuat, dan aman dari korsleting. |
| **13** | **Kontaktor Magnetik Daya** | 2x TOMZN TOCT1-25 + RC Snubber | Koil: 220V AC<br>Kontak: 25A | Sakelar daya utama pemutus dan penghubung arus ke ratusan lampu buah naga pada Blok A dan Blok B. Dilengkapi modul **RC Snubber** (kapasitor merah) untuk meredam lonjakan tegangan induktif (*arcing*) saat sakelar membuka. |
| **14** | **Unit Cadangan Daya (UPS)** | Offline / Line-Interactive UPS | Input/Output: 220V AC | Unit baterai cadangan yang menjamin mikrokontroler ESP32, sensor ZMPT, OLED, dan modem komunikasi tetap hidup tanpa jeda saat listrik PLN padam. |


---

## BAB 4: PANDUAN INSTALASI & PENGKABELAN (WIRING)

### 4.1 Persiapan Lokasi & Pemasangan Fisik Panel
1. **Posisi Pemasangan:**
   - Panel harus dipasang secara vertikal pada dinding semen atau tiang penyangga besi yang kokoh di area perkebunan.
   - Ketinggian dasar box panel minimal **1.5 meter di atas permukaan tanah** untuk menghindari percikan lumpur, genangan air saat hujan lebat, serta gangguan hewan.
   - Sangat disarankan menambahkan kanopi/atap pelindung kecil di atas panel untuk mengurangi paparan panas terik matahari langsung dan hujan badai.
2. **Jalur Masuk Kabel (Cable Gland):**
   - Seluruh kabel (input PLN, kabel beban kebun, dan kabel grounding) wajib dimasukkan melalui lubang *cable gland* di sisi **bawah** box panel.
   - Jangan pernah membuat lubang kabel di sisi atas atau samping box panel tanpa pelindung kedap air.

---

### 4.2 Skema Pengkabelan Daya Utama (AC Power Wiring)

```text
                  PLN 220V AC (Live & Neutral)
                              │
            ┌─────────────────┴─────────────────┐
            │                                   │
            ▼                                   ▼
   [ZMPT101B Sensor]                 [Terminal Block AC]
   (Deteksi Tegangan)                           │
                                      ┌─────────┴─────────┐
                                      │ (Fasa Utama)      │ (Netral)
                                      ▼                   ▼
                            [Kontaktor TOMZN]       [Terminal Netral]
                             Kontak Utama (L)             │
                                      │                   │
                        ┌─────────────┴─────────────┐     │
                        ▼                           ▼     │
                  [Kontaktor A]               [Kontaktor B]│
                        │                           │     │
                        ▼ (Fasa Terputus)           ▼     │
                  (Lewat CT A)                (Lewat CT B)│
                        │                           │     │
                        ▼                           ▼     │
                  Lampu Blok A                Lampu Blok B│
                        ▲                           ▲     │
                        └─────────────┬─────────────┘     │
                                      └───────────────────┘
```

1. **Jalur Fasa & Netral Input PLN:**
   - Kabel Fasa (Live - Hitam/Cokelat) dan Netral (Biru) dari meteran PLN masuk ke Terminal Block rel DIN.
   - Sisi primer sensor ZMPT101B dihubungkan paralel ke jalur PLN sebelum kontaktor untuk terus mendeteksi status listrik PLN.
2. **Jalur Daya Kontaktor Modular TOMZN TOCT1-25:**
   - Jalur Fasa PLN dihubungkan ke terminal input kontak utama kontaktor (Terminal 1/3).
   - Terminal keluaran kontaktor (Terminal 2/4) mengalirkan daya menuju lampu kebun Blok A dan Blok B.
   - **Koil Kontaktor (A1 - A2):** Diberi suplai 220V AC yang diputus/dihubungkan oleh modul relay kendali 5V (Active LOW) pada PCB.
3. **Pemasangan Modul RC Snubber:**
   - Modul kapasitor-resistor merah (RC Snubber) dipasang paralel langsung pada kontak daya kontaktor untuk menyerap hentakan induktif dan percikan bunga api listrik (*arcing*) saat sakelar membuka dan memutus ratusan lampu LED/CFL kebun.
4. **Pemasangan Current Transformer (CT) PZEM-004T:**
   - **CT Sensor Blok A (9A):** Diklem melingkari kabel Fasa keluaran Kontaktor A menuju beban Blok A.
   - **CT Sensor Blok B (9B):** Diklem melingkari kabel Fasa keluaran Kontaktor B menuju beban Blok B.
   - **PENTING:** Pastikan hanya **1 kabel (Fasa saja)** yang melewati lubang klem CT. Jika kabel Fasa dan Netral dimasukkan bersamaan, medan magnet akan saling meniadakan sehingga pembacaan arus menjadi 0A.
   - Sambungkan terminal tegangan AC pada modul PZEM (sekrup hijau) ke Fasa dan Netral untuk referensi perhitungan tegangan dan faktor daya (*power factor*).

---

### 4.3 Pemetaan Pinout Kendali Mikrokontroler (Low-Voltage DC)

Berikut adalah referensi koneksi kabel internal pada ESP32 DevKit V1:

| Pin ESP32 | Arah | Terhubung Ke | Keterangan / Fungsi |
|---|:--:|---|---|
| **GPIO 13** | Output | Modul Relay Kanal A (IN1) | Kendali lampu Blok A (Active LOW: LOW = ON, HIGH = OFF) |
| **GPIO 14** | Output | Modul Relay Kanal B (IN2) | Kendali lampu Blok B (Active LOW: LOW = ON, HIGH = OFF) |
| **GPIO 21** | I/O | OLED SDA & RTC DS3231 SDA | Jalur Data bus I2C (Pull-up internal/eksternal) |
| **GPIO 22** | Output | OLED SCL & RTC DS3231 SCL | Jalur Clock bus I2C |
| **GPIO 25** | Output | PZEM Blok B (RX) | Port UART Serial1 TX ESP32 |
| **GPIO 26** | Input | PZEM Blok B (TX) | Port UART Serial1 RX ESP32 |
| **GPIO 27** | Output | Modul Active Buzzer | Alarm suara peringatan pemadaman PLN & anomali beban |
| **GPIO 32** | Output | PZEM Blok A (RX) | Port UART Serial2 TX ESP32 |
| **GPIO 33** | Input | PZEM Blok A (TX) | Port UART Serial2 RX ESP32 |
| **GPIO 34** | Input | Sensor ZMPT101B (Analog OUT) | ADC1_CH6 (Input Only), pembacaan gelombang sinus AC |
| **VIN (5V)** | Daya | Output Step-Down LM2596 (5V) | Suplai utama mikrokontroler dari SMPS |
| **GND** | Ground | Common Ground DC | Titik temu ground ESP32, Sensor, Step-Down, dan Relay |

---

### 4.4 Pengujian Koneksi Pra-Penyalaan (Cold Check)
Sebelum menyalakan sakelar daya PLN atau UPS untuk pertama kali:
1. Periksa kekencangan seluruh baut pada terminal block dan terminal kontaktor menggunakan obeng berisolasi.
2. Gunakan multimeter pada mode *Continuity / Beep* untuk memastikan **tidak ada korsleting antara Fasa dan Netral** pada terminal AC.
3. Pastikan kabel kuning-hijau (PE) terhubung kuat antara bodi rel DIN ke grounding rod tanah.
4. Pastikan steker kabel SMPS tertancap sempurna pada stop kontak Schneider.

---

## BAB 5: PROSEDUR PENGOPERASIAN

### 5.1 Prosedur Menyalakan Alat (Power-On Sequence)
Untuk mencegah lonjakan arus sesaat (*inrush current*) pada rangkaian elektronik, ikuti urutan penyalaan berikut:

1. **Langkah 1 (Nyalakan Cadangan UPS):**
   - Tekan tombol Power pada unit UPS hingga berbunyi *bip* satu kali dan lampu indikator baterai/daya menyala.
   - Amati lampu indikator LED pada SMPS 12V dan papan PCB utama yang akan mulai menyala.
2. **Langkah 2 (Nyalakan Sumber Utama PLN):**
   - Naikkan tuas MCB PLN utama di panel distribusi kebun.
   - Sensor ZMPT101B akan mendeteksi tegangan jala-jala PLN dalam hitungan milidetik.
3. **Langkah 3 (Verifikasi Layar OLED):**
   - Amati layar OLED 0.96". Layar akan menampilkan proses boot, inisialisasi WiFi, jam RTC, dan membaca status PLN (`Source: PLN`).

---

### 5.2 Antarmuka Layar OLED Lokal

Layar OLED menampilkan 7 baris informasi status operasional terkini tanpa perlu membuka smartphone:

```text
┌──────────────────────────────┐
│           18:30:15           │ ◄── Baris 1: Jam Real-Time (RTC DS3231)
│ Wifi: online 45ms            │ ◄── Baris 2: Status WiFi & Latensi Ping Internet
│ Source: PLN                  │ ◄── Baris 3: Sumber Daya Aktif (PLN / UPS)
│ Blok A: 1250W                │ ◄── Baris 4: Konsumsi Daya Nyata Blok A
│ Standby / Normal             │ ◄── Baris 5: Diagnosa Kesehatan Beban Blok A
│ Blok B: 1240W                │ ◄── Baris 6: Konsumsi Daya Nyata Blok B
│ Standby / Normal             │ ◄── Baris 7: Diagnosa Kesehatan Beban Blok B
└──────────────────────────────┘
```

**Arti Indikator Status:**
- **Baris 1 (Jam):** Jika muncul `--:--:--`, baterai RTC habis atau komunikasi I2C terganggu.
- **Baris 2 (WiFi):**
  - `Wifi: online [xx]ms`: Terkoneksi ke router dan internet aktif dengan latensi tertera.
  - `Wifi: No Internet`: Terhubung ke WiFi lokal/router kebun, tetapi jaringan internet/kuota modem mati.
  - `Wifi: PORTAL`: Mode Access Point aktif untuk konfigurasi nama WiFi.
  - `Wifi: offline`: Tidak tersambung ke jaringan nirkabel.
- **Baris 3 (Source):**
  - `Source: PLN`: Beroperasi normal dengan suplai jaringan listrik PLN.
  - `Source: UPS`: Listrik PLN padam! Sistem kendali berjalan menggunakan cadangan baterai UPS.
- **Baris 4–7 (Blok A & B):** Menampilkan besar daya aktif yang mengalir ke lampu kebun beserta status hasil diagnosa pohon keputusan (*decision tree*).

---

### 5.3 Konfigurasi Awal Jaringan WiFi (Portal Energa-AP)
Jika panel baru dipasang di kebun atau jika SSID/password router WiFi kebun diganti:

```
           HP / LAPTOP                        BOX PANEL ENERGA
                │                                    │
   1. Hubungkan WiFi ke "Energa-AP"                 │
      (Open / Tanpa Password)                        │
                ├───────────────────────────────────►│ (ESP32 Mode AP)
                │                                    │
   2. Buka Browser: 192.168.4.1                      │
                ├───────────────────────────────────►│ (Buka Web Portal)
                │                                    │
   3. Pilih Menu "Configure WiFi"                    │
      Pilih SSID & Masukkan Password                 │
                ├───────────────────────────────────►│
                │                                    │
   4. Klik "Save"                                    │
                ├───────────────────────────────────►│ (Simpan ke NVS)
                │                                    ▼
                │                          ESP32 Restart Otomatis
                │                          Tersambung ke Router Kebun!
```

1. Saat ESP32 tidak menemukan WiFi yang tersimpan dalam 30 detik (atau jika diperintahkan reset dari aplikasi), OLED akan menampilkan panduan `== SETUP WIFI ==`.
2. Buka menu pengaturan WiFi pada smartphone atau laptop Anda, lalu sambungkan ke jaringan bernama **`Energa-AP`** (Open / tidak memerlukan password).
3. Buka peramban (*web browser*) seperti Chrome/Safari, lalu ketik alamat IP: **`192.168.4.1`**.
4. Di halaman web yang muncul, pilih tombol **Configure WiFi**.
5. Pilih nama WiFi kebun Anda dari daftar yang terdeteksi, masukkan kata sandi WiFi, lalu klik **Save**.
6. ESP32 akan menyimpan konfigurasi ke memori non-volatile (NVS) dan restart otomatis. Layar OLED akan segera menampilkan status `Wifi: online`.

---

### 5.4 Pengoperasian via Mobile App (Energa App)
Aplikasi mobile **Energa** (berbasis Flutter) memungkinkan monitoring dan kontrol jarak jauh:
1. **Kendali Manual:**
   - Tombol sakelar sentuh untuk menyalakan atau mematikan lampu Blok A dan Blok B kapan saja dari mana saja.
2. **Jadwal Penyinaran Otomatis (Scheduler):**
   - Atur jam lampu menyala (misal: 18:00) dan jam padam (misal: 22:00 atau 04:00). Jadwal akan disinkronkan ke memori internal ESP32 dan RTC sehingga tetap beroperasi meskipun koneksi internet terputus di malam hari.
3. **Konfigurasi Diagnosa Anomali Beban:**
   - Pada menu pengaturan beban per blok, masukkan:
     - `expected_power`: Total daya nominal lampu saat seluruhnya menyala (contoh: 1500 Watt).
     - `lamp_count`: Jumlah total unit lampu yang terpasang pada blok tersebut (contoh: 100 lampu).
     - `tolerance_percent`: Toleransi fluktuasi daya yang diizinkan (default: 15%).

---

### 5.5 Prosedur Mematikan Alat (Safe Shutdown Sequence)
1. Matikan beban lampu kebun melalui aplikasi mobile (atau pastikan kontaktor dalam kondisi OFF).
2. Turunkan tuas MCB PLN utama untuk mematikan jalur tegangan tinggi 220V AC.
3. Tekan dan tahan tombol Power pada unit UPS hingga indikator lampu padam dan sistem kontrol mati total.


---

## BAB 6: SISTEM DETEKSI ANOMALI & ALARM

### 6.1 Arsitektur Diagnosa Cerdas (Firmware-First Decision Tree)
Sistem **Energa** dilengkapi mesin diagnosa kesehatan instalasi lampu otomatis yang berjalan langsung di mikrokontroler ESP32 setiap 1 detik. Evaluasi ini bekerja secara mandiri (*edge computing*), sehingga kerusakan atau pemadaman listrik langsung terdeteksi dan ditampilkan di layar OLED tanpa bergantung pada koneksi internet.

Pohon keputusan (*decision tree*) mengevaluasi status berdasarkan urutan prioritas (*precedence*) berikut:

```
[Mulai Evaluasi Tiap 1 Detik]
   │
   ├─► 1. Apakah data sensor PZEM valid & terbaca?
   │      └── TIDAK ──► Status: SENSOR ERROR (SensorError)
   │
   ├─► 2. Apakah status perintah lampu = OFF (Relay Mati)?
   │      ├── Daya > 2.0W atau Arus > 0.05A ──► Status: KEBOCORAN RELAY (RelayLeakage)
   │      └── Daya ≤ 2.0W ───────────────────► Status: STANDBY NORMAL (NormalStandby)
   │
   └─► 3. Status perintah lampu = ON (Relay Hidup):
          ├── Tegangan < 50V AC  ────────────► Status: PLN PADAM (PlnOutage)
          ├── Tegangan < 190V AC ────────────► Status: TEGANGAN DROP (UnderVoltage)
          ├── Tegangan > 245V AC ────────────► Status: TEGANGAN LEBIH (OverVoltage)
          │
          └── Tegangan Normal (190V - 245V):
                 ├── Daya < 2.0W ────────────► Status: JALUR BEBAN PUTUS (CircuitBroken)
                 ├── Daya < Batas Bawah Tol ─► Status: LAMPU MATI SEBAGIAN (PartialLampFail)
                 ├── Daya > Batas Atas Tol ──► Status: BEBAN LEBIH / LONJAKAN (OverLoad)
                 └── Daya dalam rentang tol ──► Status: BEROPERASI NORMAL (NormalActive)
```

#### Rincian 10 Kondisi Diagnosa Sistem:

1. **`SENSOR ERROR` (Sensor Tidak Terbaca):**
   - **Indikasi:** Modul PZEM-004T tidak merespons perintah Modbus UART atau kabel sinyal terputus.
   - **Tindakan Sistem:** Menampilkan pesan error di OLED dan mengabaikan nilai daya yang basi (*stale data*).
2. **`RELAY LEAKAGE` (Kebocoran Arus Saat Lampu Mati):**
   - **Indikasi:** Sakelar kontrol berstatus OFF, namun sensor mendeteksi arus > 0.05A atau daya > 2.0W mengalir ke kebun.
   - **Penyebab:** Kontak kontaktor lengket akibat lonjakan arus (*welded contact*) atau terjadi kebocoran isolasi kabel ke tanah.
3. **`STANDBY NORMAL` (Siang Hari / Standby):**
   - **Indikasi:** Lampu dalam kondisi OFF terjadwal dan tidak ada arus yang mengalir. Sistem siap beroperasi.
4. **`PLN OUTAGE` (Listrik PLN Padam):**
   - **Indikasi:** Sensor ZMPT101B mendeteksi hilangnya gelombang AC PLN (RMS < 150) atau tegangan PZEM < 50V saat jadwal lampu menyala.
   - **Tindakan Sistem:** Mengaktifkan alarm buzzer periodik dan mengirimkan alert pemadaman ke smartphone petani via daya cadangan UPS.
5. **`UNDER VOLTAGE` (Tegangan Turun / Drop):**
   - **Indikasi:** Tegangan jala-jala PLN drop di bawah 190V AC.
   - **Dampak:** Dapat menyebabkan lampu berkedip (*flicker*) atau merusak *driver* lampu LED.
6. **`OVER VOLTAGE` (Tegangan Lebih / Lonjakan):**
   - **Indikasi:** Tegangan jala-jala PLN melonjak di atas 245V AC.
   - **Dampak:** Berisiko memperpendek umur lampu atau menyebabkan bohlam lampu terbakar/putus.
7. **`CIRCUIT BROKEN` (Jalur Instalasi Putus Total):**
   - **Indikasi:** Kontaktor dalam kondisi ON, tegangan PLN normal, namun daya yang terukur < 2.0W.
   - **Penyebab:** Kabel utama instalasi kebun putus (tertumpu dahan/pohon, gigitan hewan) atau sekring pengaman putus.
8. **`PARTIAL LAMP FAIL` (Lampu Mati Sebagian):**
   - **Indikasi:** Daya terukur lebih rendah dari ambang batas bawah toleransi:  
     $$\text{Daya Terukur} < \text{Daya Nominal} \times \left(1 - \frac{\text{Toleransi \%}}{100}\right)$$
   - **Fitur Cerdas:** Firmware menghitung estimasi jumlah bohlam lampu yang putus berdasarkan daya per unit lampu:
     $$\text{Estimasi Lampu Mati} = \left\lceil \frac{\text{Daya Nominal} - \text{Daya Terukur}}{\text{Daya per Lampu}} \right\rceil$$
     Tampilan OLED akan langsung memperlihatkan misalnya: `3 Lampu Mati`.
9. **`OVER LOAD` (Beban Berlebih / Surging):**
   - **Indikasi:** Daya terukur melampaui batas toleransi atas. Menandakan adanya beban liar tambahan yang disambung ke jalur lampu atau penurunan impedansi akibat korsleting sebagian.
10. **`NORMAL ACTIVE` (Beroperasi Normal):**
    - **Indikasi:** Seluruh lampu menyala stabil dan daya berada di dalam rentang ambang batas toleransi yang ditetapkan.

---

### 6.2 Pola Alarm Audio Buzzer (Active Buzzer GPIO 27)

Buzzer bertindak sebagai pemberi peringatan lokal bagi teknisi atau penjaga kebun yang berada di dekat panel:

| Pola Suara Buzzer | Kondisi Pemicu | Arti Peringatan & Tindakan |
|---|---|---|
| **Beep Periodik**<br>(500 ms Bunyi, 2000 ms Senyap) | Listrik PLN Padam (`PLN_OUTAGE`) | **Peringatan Pemadaman:** Sistem berjalan dengan cadangan baterai UPS. Periksa sumber PLN kebun atau genset. |
| **Beep Cepat Berulang**<br>(100 ms Bunyi, 100 ms Senyap) | Beban Lebih Kritis (`OVER_LOAD`) | **Bahaya Beban Berlebih:** Segera periksa apakah ada korsleting pada instalasi kabel kebun. |
| **Beep Tunggal Singkat**<br>(1x 200 ms) | Startup / Sinkronisasi Berhasil | Indikasi booting selesai dan ESP32 berhasil tersambung ke WiFi & Firebase. |
| **Senyap (Mute)** | Kondisi Normal / Standby | Operasional normal tanpa adanya anomali kritis. |

---

## BAB 7: PEMELIHARAAN BERKALA (MAINTENANCE)

Untuk menjaga performa sistem otomasi Energa agar dapat beroperasi stabil selama bertahun-tahun di lingkungan perkebunan, lakukan jadwal perawatan preventif berikut:

### 7.1 Tabel Jadwal Pemeliharaan Preventif

| Frekuensi | Komponen yang Diperiksa | Prosedur Pemeliharaan |
|---|---|---|
| **Harian** | Layar OLED & Indikator LED | Periksa jam RTC, pastikan status `Wifi: online`, dan pastikan tidak muncul label error pada Blok A/B. |
| **Mingguan** | Aplikasi Mobile Energa | Pantau grafik konsumsi kWh mingguan dan pastikan tidak ada notifikasi lampu mati sebagian. |
| **Bulanan** | Kebersihan Dalam Box Panel | Buka box panel (pastikan cuaca kering), bersihkan debu halus dengan kuas kering lembut atau mini blower. Periksa apakah ada semut/serangga di dekat PCB. |
| **Bulanan** | Unit Cadangan Baterai (UPS) | Uji ketahanan baterai UPS: Matikan MCB PLN utama sesaat pada siang hari, pastikan ESP32 dan OLED tetap menyala stabil minimal 10 menit tanpa gangguan. |
| **Per 6 Bulan** | Baut Terminal Rel DIN & Kontaktor | Matikan seluruh daya (PLN + UPS). Kencangkan seluruh sekrup terminal kabel menggunakan obeng berisolasi. Getaran dan perubahan suhu kebun dapat melonggarkan baut kabel. |
| **Per 6 Bulan** | Modul RC Snubber | Periksa fisik kapasitor merah RC snubber. Pastikan tidak melepuh, retak, atau hangus akibat lonjakan petir. |
| **Per 6 Bulan** | Resistansi Grounding (PE) | Ukur tahanan tanah grounding rod menggunakan *Earth Tester*. Pastikan nilai tetap di bawah 5 Ohm. Siram air garam di sekitar grounding rod jika tanah mengering. |
| **Per 1–2 Tahun** | Baterai Kancing RTC (CR2032) | Ganti baterai coin cell CR2032 pada modul DS3231 jika jam mulai reset ke `00:00:00` saat panel dinyalakan dari kondisi mati total. |

---

### 7.2 Prosedur Penggantian Komponen Cadangan (Spare Parts Replacement)
Jika terjadi kerusakan perangkat keras:
1. **Mengganti ESP32:** Lepaskan modul ESP32 lama dari *female socket header* pada PCB dengan mengangkatnya secara perlahan dari kedua sisi secara seimbang. Pasang board pengganti dengan orientasi pin yang sama persis (port micro USB menghadap ke bawah/luar sesuai layout PCB).
2. **Mengganti Kontaktor Daya TOMZN:** Lepaskan klip pengunci rel DIN di bagian bawah kontaktor menggunakan obeng minus. Lepaskan kabel satu per satu dan beri label nomor terminal kabel agar tidak tertukar saat memasang kontaktor baru.

---

## BAB 8: PANDUAN PEMECAHAN MASALAH (TROUBLESHOOTING)

Gunakan tabel matriks di bawah ini untuk mendiagnosis dan menangani kendala teknis yang umum terjadi di lapangan:

### 8.1 Matriks Diagnosis Masalah Cepat

| No | Gejala / Indikasi Masalah | Kemungkinan Penyebab | Tindakan Solusi Perbaikan |
|:--:|---|---|---|
| **1** | **Seluruh sistem mati total** (Layar OLED mati, lampu LED PCB & SMPS tidak menyala). | • Steker kabel SMPS lepas.<br>• Unit UPS mati atau baterai habis total.<br>• Sakelar MCB PLN utama trip/putus. | 1. Periksa tuas MCB PLN induk di luar panel.<br>2. Periksa steker hitam pada stop kontak Schneider.<br>3. Tekan tombol Power switch pada unit UPS. |
| **2** | **Layar OLED menampilkan jam `--:--:--`**. | • Baterai CR2032 pada RTC habis.<br>• Jalur bus I2C (GPIO 21/22) longgar.<br>• Modul RTC DS3231 rusak. | 1. Periksa soket modul RTC pada PCB.<br>2. Ganti baterai kancing CR2032 dengan yang baru.<br>3. Lakukan restart sistem. Jam akan otomatis tersinkronisasi via NTP saat ada internet. |
| **3** | **OLED menampilkan `Wifi: PORTAL`**. | • Kredensial WiFi di memori terhapus.<br>• Router/modem WiFi kebun dalam keadaan mati atau berada di luar jangkauan sinyal. | 1. Pastikan router/modem 4G kebun menyala.<br>2. Sambungkan smartphone ke WiFi `Energa-AP` (tanpa password).<br>3. Buka browser `192.168.4.1`, pilih nama WiFi dan masukkan kata sandi yang benar. |
| **4** | **OLED menampilkan `Wifi: No Internet`**. | • Terhubung ke router, namun router tidak memiliki koneksi ke internet (kuota modem habis / sinyal LTE hilang). | 1. Periksa kuota data pada kartu SIM modem kebun.<br>2. Lakukan restart pada modem 4G kebun.<br>3. Sistem tetap menjalankan jadwal penyinaran secara offline via jam RTC. |
| **5** | **Nilai daya (Watt) selalu 0W padahal lampu kebun menyala terang**. | • Klem CT PZEM-004T terbalik atau menjepit kabel Fasa dan Netral sekaligus.<br>• Kabel sinyal klem CT ke modul PZEM lepas.<br>• Terminal tegangan AC PZEM (sekrup hijau) tidak terhubung ke 220V. | 1. Pastikan klem CT **hanya melingkari 1 kabel Fasa** beban lampu.<br>2. Periksa sekrup terminal CT pada modul PZEM hijau.<br>3. Ukur dengan multimeter apakah ada tegangan 220V pada terminal sekrup hijau PZEM. |
| **6** | **Status diagnosa menampilkan `Sensor Error`**. | • Kabel data UART antara ESP32 dan modul PZEM terlepas.<br>• Modul PZEM-004T mengalami freeze komunikasi Modbus. | 1. Periksa kabel jumper GPIO 33/32 (Blok A) atau GPIO 26/25 (Blok B).<br>2. Matikan daya kontrol dan hidupkan kembali untuk me-reset mikrokontroler PZEM. |
| **7** | **Lampu kebun tidak menyala padahal aplikasi menunjukkan status ON**. | • MCB PLN jalur beban lampu trip.<br>• Koil kontaktor TOMZN tidak mendapatkan suplai 220V.<br>• Modul relay 5V pada PCB tidak terpicu (rusak optocoupler). | 1. Periksa apakah kontaktor berbunyi *klik* tegas saat tombol sakelar aplikasi ditekan.<br>2. Jika tidak ada suara klik, periksa kabel sinyal GPIO 13/14 ke modul relay.<br>3. Periksa tegangan pada terminal koil A1-A2 kontaktor. |
| **8** | **Status menampilkan `Lampu Mati Sebagian`**. | • Sebagian bohlam lampu di kebun putus atau fitting lampu korosi terkena hujan. | 1. Periksa baris status OLED untuk melihat estimasi jumlah lampu mati (misal: `4 Lampu Mati`).<br>2. Lakukan inspeksi visual di deretan tiang kebun pada malam hari untuk mengganti bohlam yang padam. |
| **9** | **Buzzer berbunyi bip periodik terus menerus**. | • Listrik PLN kebun sedang padam (`PLN_OUTAGE`). | • Ini adalah peringatan normal bahwa sistem bekerja dalam mode darurat baterai UPS.<br>• Buzzer akan otomatis berhenti saat pasokan listrik PLN menyala kembali. |
| **10** | **Kontaktor bergetar keras (*chattering*) atau mendengung berisik**. | • Tegangan suplai PLN drop di bawah ambang batas kerja koil kontaktor (< 175V).<br>• Terdapat kotoran debu pada celah magnet inti besi kontaktor. | 1. Ukur tegangan PLN menggunakan multimeter.<br>2. Bersihkan kontaktor dengan semprotan *contact cleaner* (dalam kondisi daya mati total).<br>3. Ganti unit kontaktor jika getaran tidak hilang. |

---

### 8.2 Prosedur Reset Konfigurasi Jaringan WiFi (Factory Reset WiFi)
Jika Anda perlu mengganti jaringan WiFi tanpa membuka box panel:
1. **Metode Aplikasi Mobile:**  
   Buka menu **Pengaturan Perangkat** di aplikasi Energa, lalu pilih opsi **Reset Pengaturan WiFi**. ESP32 akan menghapus kredensial WiFi dari memori NVS dan otomatis restart ke mode Access Point (`Energa-AP`).
2. **Metode Otomatis Lapangan:**  
   Jika router WiFi lama dimatikan atau dipindahkan, nyalakan panel Energa. Setelah 30 detik gagal tersambung, panel akan otomatis beralih ke mode Access Point (`Energa-AP`) dan menampilkan panduan setup pada layar OLED.

---

### 8.3 Dukungan Teknis & Kontak Bantuan

Jika kendala operasional belum teratasi setelah mengikuti panduan pemecahan masalah di atas:
- **Tim Pengembang Hardware & IoT:** Magang Smart Farming & Tim Teknis Energa
- **Repositori & Dokumentasi Firmware:** `kontrol-lampu-buah-naga` (PlatformIO / ESP32)
- **Repositori Aplikasi Mobile:** `smartnaga_app` (Flutter Framework)
