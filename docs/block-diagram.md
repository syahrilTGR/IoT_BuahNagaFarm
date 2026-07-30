# Blok Diagram Sistem Kontrol Pencahayaan Nirkabel Buah Naga

## Ringkasan Sistem
Sistem terdiri dari tiga bagian utama:
1. **Node Sensor & Kontrol** (ESP32 dengan periferal)
2. **Backend** (Firebase Realtime Database + Firestore)
3. **Frontend** (Aplikasi Mobile)

## Komponen Node Sensor & Kontrol

| Komponen | Fungsi | Keterangan |
|----------|--------|------------|
| **ESP32** | Mikrokontroler pusat | Mengendalikan semua I/O, komunikasi WiFi, dan logika pemrosesan |
| **PZEM-004T v3.0** (2 unit) | Pengukuran energi listrik | Mengukur tegangan (V), arus (A), daya (W), dan energi (kWh) untuk Blok A dan Blok B |
| **Current Transformer (CT)** | Sensor arus untuk PZEM | Dipasang di kabel fasa menuju beban lampu |
| **AC Detector (Optocoupler)** | Deteksi kehadiran listrik secara instan | Mendeteksi apakah ada tegangan AC di kabel beban (lihat bagian detail di bawah) |
| **Logic Level Converter** | Penyesuaian level logika | Mengubah 3.3V ESP32 ke 5V untuk PZEM dan sebaliknya |
| **Relay 2-Channel** | Pengendalian lampu | Mengaktifkan/mematikan pasokan listrik ke lampu Blok A dan Blok B |
| **OLED LCD (0.96" 128x64 I2C)** | Tampilan status lokal | Menampilkan tegangan, arus, daya, dan status lampu |
| **Buzzer** | Alarm auditory | Bunyi ketika terdeteksi gangguan lampu |
| **USB Modem LTE** | Konektivitas internet cadangan | Jika WiFi tidak tersedia, menggunakan koneksi seluler |
| **Power Supply 12V 5A + Step Down 5V** | Pengadaan daya | Mengubah listrik PLN 220V menjadi 12V untuk sistem dan turun ke 5V untuk logika |

---

## Detail Komponen: AC Detector (Optocoupler)

### Fungsi
AC Detector berfungsi untuk mendeteksi **kehadiran tegangan AC 220V pada kabel beban** secara instan (HIGH/LOW). Komponen ini memberikan feedback cepat apakah arus listrik masih mengalir ke lampu atau tidak, tanpa harus menunggu bacaan dari PZEM yang lebih lambat.

### Posisi dalam Sistem
Dalam blok diagram, terdapat **1 AC Detector** di posisi strategis:

| Posisi | Letak | Fungsi |
|--------|-------|--------|
| **AC Detector (input side)** | Setelah MCB, sebelum UPS/Power Supply | Mendeteksi pemadaman dari PLN secara instan. Jika HIGH → PLN padam → sistem switch ke UPS/baterai |

> **Catatan:** AC Detector di **sisi beban (setelah relay)** **tidak diperlukan**, karena PZEM-004T sudah mengukur daya/energi di beban tersebut. Deteksi kondisi lampu (padam/rusak) sudah ditangani oleh pembacaan daya dari PZEM.

### Cara Kerja
```
[PLN 220V] → [Resistor pembatas] → [Optocoupler] → [Logic Output 3.3V/5V] → [ESP32 GPIO]
```
- Optocoupler bekerja dengan prinsip **isolasi galvanik** — input AC dan output logic tidak tersentuh secara fisik, hanya dikirimkan melalui cahaya internal.
- Ketika tegangan AC ada → LED internal optocoupler menyala → transistor output ON → output LOW.
- Ketika tegangan AC hilang → LED mati → transistor OFF → output HIGH (dengan pull-up resistor).

### Peran dalam Sistem
| Kondisi | PZEM (power) | AC Detector (PLN) | Interpretasi |
|---------|--------------|-------------------|--------------|
| PLN normal, Lampu ON | Normal | LOW (PLN ada) | ✅ Normal |
| PLN padam, Lampu OFF | Kurang dari normal | HIGH (PLN padam) | ❌ Pemadaman PLN |
| PLN normal, Lampu padam | Kurang dari normal | LOW (PLN ada) | ❌ Gangguan Lampu |
| PLN normal, Lampu rusak | Lebih dari normal | LOW (PLN ada) | ⚠️ Gangguan Lampu |
| Relay OFF (normal) | Normal | LOW (PLN ada) | ✅ Normal |

**Keunggulan kombinasi PZEM + AC Detector:**
- PZEM lambat (~5 detik per bacaan) tapi akurat untuk pengukuran energi.
- AC Detector instan (~ms) tapi hanya tahu ada/tidak ada arus.
- Keduanya bersama-sama memberikan **deteksi gangguan yang cepat dan akurat**.

### Komponen Optocoupler yang Direkomendasikan
| Opsi | Tipe | Harga | Keterangan |
|------|------|-------|------------|
| **H11AA1** | Dual Optocoupler (AC input) | Rp 5.000–10.000 | Khusus untuk AC input, zero-crossing detection, 2 channel |
| **4N35** | Single Optocoupler | Rp 2.000–5.000 | Perlu resistor dan dioda penyearah tambahan |
| **EL817** | Single Optocoupler | Rp 1.500–3.000 | Paling murah, perlu additional rectifier circuit |

**Rekomendasi: H11AA1** — dirancang khusus untuk input AC, cukup 1 IC untuk 2 channel (Blok A dan Blok B).

### Wiring (H11AA1 ke ESP32)
```
                    H11AA1
               ┌────────────┐
  Fasa lampu ──┤1  AC IN    │
  Nol lampu ───┤2  AC IN    │
               │            │
  ESP32 GPIO4──┤3  OUT 1    │
  ESP32 GPIO5──┤4  OUT 2    │
  3.3V ────────┤5  VCC      │
               │6  GND      │
               └────────────┘
                 │
              GND ESP32

Resistor: 2 × 100kΩ (series antara Fasa/Nol ke pin opto)
Pull-up: 2 × 10kΩ (pin output ke 3.3V)
```

## Alur Kerja Sistem

### 1. Inisialisasi
- ESP32 melakukan inisialisasi periferal (UART untuk PZEM, I2C untuk OLED, GPIO untuk relay dan buzzer).
- Menghubungkan ke WiFi (atau USB Modem LTE sebagai cadangan).
- Menginisialisasi koneksi ke Firebase Realtime Database.

### 2. Pembacaan Sensor & Kontrol
- **Setiap 100ms**:
  - Membaca data dari PZEM-004T untuk Blok A dan Blok B melalui UART.
  - Membaca status AC Detector untuk deteksi instan kehilangan listrik.
  - Memeriksa perintah kontrol dari Firebase (path `/lamp/control/a` dan `/lamp/control/b`).

- **Setiap 5 detik**:
  - Mengirimkan data energi (tegangan, arus, daya, kWh) dan timestamp ke Firebase:
    - `/energy/block_a`
    - `/energy/block_b`

- **Setiap 10 detik**:
  - Mengirimkan status lampu (ON/OFF) ke Firebase:
    - `/lamp/status/a`
    - `/lamp/status/b`

### 3. Deteksi Gangguan Lampu
- Menggunakan data daya (W) dari PZEM:
  - Jika lampu dalam keadaan ON tetapi daya < 2W → dianggap **padam/hilang**.
  - Jika daya > 15W (meski sebesar dari nilai normal) → dianggap **rusak**.
- Jika terdeteksi gangguan:
  - Mengirimkan notifikasi ke Firebase ke path `/alerts` dengan timestamp, jenis gangguan, dan pesan.
  - Mengaktifkan buzzer sebagai alarm lokal.

### 4. Kontrol Lampu dari Aplikasi Mobile
- Pengguna mengirimkan perintah ON/OFF melalui aplikasi mobile.
- Perintah ditulis ke Firebase di path:
  - `/lamp/control/a` (untuk Blok A)
  - `/lamp/control/b` (untuk Blok B)
- ESP32 mendengarkan perubahan pada path tersebut melalui Firebase stream dan mengendalikan relay sesuai.

### 5. Monitoring Lokal di OLED
- OLED menampilkan secara real-time:
  - Tegangan dan arus tiap blok.
  - Daya konsumsi dan energi terakumulasi.
  - Status lampu (ON/OFF).
  - Status koneksi WiFi/LTE.

### 6. Backup dan Sinkronisasi Data
- Data energi yang dikirim ke Realtime Database secara periodik secara otomatis ditransfer ke Firestore melalui Firebase Cloud Function untuk penyimpanan historical.
- Aplikasi mobile dapat melakukan ekspor manual data historis dalam format CSV atau PDF.

## Aliran Data

```
[PLN 220V] 
   ↓
[MCB 16A] → [Fasa & Netral]
   ↓
[AC Detector (untuk deteksi pemadaman)] ──→ GPIO (AC Detector)   (sebelum UPS)
   ↓
[Power Supply 12V 5A] → [Step Down 5V] → [ESP32 Power]
   ↓
[CT] ──→ [PZEM-004T] ←── UART ──→ [ESP32]
   ↓                     ↑
[Relay] ←── GPIO ───────┘        (Kontrol lampu)
   ↓
[Lampu Blok A/B]
   ↓
[OLED LCD] ←── I2C ──→ [ESP32] (tampilan lokal)
   ↓
[Buzzer] ←── GPIO ──→ [ESP32] (alarm)
   ↓
[WiFi / USB Modem LTE] ──→ Internet ──→ [Firebase]
   ↓
[Mobile App] ←── HTTPS/API ──→ [Firebase]
```

## Keunggulan Desain
- **Real-time monitoring dan kontrol** melalui Firebase Realtime Database.
- **Deteksi pemadaman PLN instan** menggunakan AC Detector di sisi input sebelum UPS.
- **Deteksi gangguan lampu** berdasarkan pembacaan daya PZEM-004T (tidak perlu AC Detector di sisi beban).
- **Komunikasi nirkabel** memungkinkan pengendalian dari jarak jauh melalui aplikasi mobile.
- **Pemborosan energi diminimalkan** karena sistem hanya menyuplai listrik ke lampu ketika diperlukan dan memberikan feedback konsumsi.
- **Sistem modular** sehingga dapat diperluas menjadi Wireless Sensor Network (WSN) multi-node di masa depan.

## Referensi Gambar
Blok diagram lengkap dapat dilihat di `docs/block-diagram.png` (jika tersedia) atau mengacu pada deskripsi di atas.