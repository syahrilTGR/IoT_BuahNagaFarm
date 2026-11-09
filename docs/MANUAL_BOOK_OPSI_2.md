# BUKU PANDUAN PENGGUNA (MANUAL BOOK — OPSI 2)
## Energa — Sistem Kontrol & Monitoring Pencahayaan Buah Naga Berbasis IoT

**Versi Dokumen:** 2.0 (Opsi 2: Sudut Pandang Pengguna / User POV)  
**Tanggal Rilis:** 2026-11-09  
**Target Pembaca:** Pemilik Kebun, Petani Buah Naga, dan Pengelola Lapangan  
**Aplikasi Pendukung:** Energa Mobile App (Android / iOS)  

---

```
  ================================================================================
   🌿 ENERGA: PANDUAN PENGOPERASIAN CERDAS KEBUN BUAH NAGA OFF-SEASON 🌿
  ================================================================================
   Buku panduan ini dirancang khusus untuk memudahkan pemilik kebun dan petani 
   dalam mengoperasikan alat, mengatur jadwal penyinaran lampu malam, memahami 
   tampilan layar monitor, membaca alarm, serta menangani gangguan lampu di kebun.
```

---

## DAFTAR ISI

- [BAB 1: MENGENAL SISTEM ENERGA & MANFAATNYA](#bab-1-mengenal-sistem-energa--manfaatnya)
  - 1.1 Mengapa Kebun Buah Naga Membutuhkan Energa?
  - 1.2 Keuntungan Utama bagi Pemilik Kebun
  - 1.3 Pembagian Dua Blok Lampu (Blok A & Blok B)
  - 1.4 Keamanan Daya Ganda (Listrik PLN & Baterai Cadangan UPS)
- [BAB 2: MENGENAL BAGIAN-BAGIAN BOX PANEL DI KEBUN](#bab-2-mengenal-bagian-bagian-box-panel-di-kebun)
  - 2.1 Tampak Luar Box Panel
  - 2.2 Bagian Dalam Panel yang Perlu Diketahui Pengguna
  - 2.3 Perangkat Tambahan (Modem 4G & Baterai UPS)
- [BAB 3: PANDUAN KESELAMATAN KERJA (K3) PETANI](#bab-3-panduan-keselamatan-kerja-k3-petani)
  - 3.1 Aturan Penting Kelistrikan Kebun
  - 3.2 Peringatan Khusus: Baterai Cadangan UPS Tetap Menyala!
  - 3.3 Tindakan Cepat Saat Terjadi Kondisi Darurat
- [BAB 4: CARA MENYALAKAN & MEMATIKAN ALAT (SOP HARIAN)](#bab-4-cara-menyalakan--mematikan-alat-sop-harian)
  - 4.1 Prosedur Menyalakan Alat Pertama Kali
  - 4.2 Tanda Alat Sudah Siap Beroperasi Normal
  - 4.3 Prosedur Mematikan Panel Secara Aman (Shutdown)
- [BAB 5: CARA MENGHUBUNGKAN ALAT KE WIFI / MODEM KEBUN LEWAT HP](#bab-5-cara-menghubungkan-alat-ke-wifi--modem-kebun-lewat-hp)
  - 5.1 Kapan Perlu Mengatur WiFi?
  - 5.2 Langkah Cepat Sambung WiFi (Hanya 3 Menit)
  - 5.3 Cara Memastikan Koneksi Sudah Berhasil
- [BAB 6: CARA MEMBACA LAYAR MONITOR DI BOX PANEL](#bab-6-cara-membaca-layar-monitor-di-box-panel)
  - 6.1 Susunan Tampilan 7 Baris di Layar
  - 6.2 Memahami Arti Informasi di Layar
  - 6.3 Contoh Tampilan Saat Semua Lampu Sehat
  - 6.4 Contoh Tampilan Saat Ada Lampu yang Putus
- [BAB 7: ARTI BUNYI ALARM (BUZZER) & TINDAKAN ANDA](#bab-7-arti-bunyi-alarm-buzzer--tindakan-anda)
  - 7.1 Bunyi Beep Singkat (1 Kali)
  - 7.2 Bunyi Beep Panjang Terputus-Putus (Alarm Listrik PLN Padam)
  - 7.3 Bunyi Beep Cepat Rapat (Alarm Beban Berlebih / Bahaya)
- [BAB 8: PANDUAN MENGGUNAKAN APLIKASI DI SMARTPHONE (ENERGA APP)](#bab-8-panduan-menggunakan-aplikasi-di-smartphone-energa-app)
  - 8.1 Membuka Aplikasi & Melihat Status Kebun Real-Time
  - 8.2 Menyalakan & Mematikan Lampu Manual Lewat HP
  - 8.3 Mengatur Jadwal Lampu Otomatis (Timer Penyinaran Malam)
  - 8.4 Mengisi Data Konfigurasi Lampu (PENTING untuk Deteksi Kerusakan)
  - 8.5 Memantau Pemakaian Energi & Perkiraan Tagihan Listrik
- [BAB 9: PANDUAN MENGATASI GANGGUAN DI LAPANGAN (TROUBLESHOOTING)](#bab-9-panduan-mengatasi-gangguan-di-lapangan-troubleshooting)
  - 9.1 Status "Listrik Padam" (PLN Mati)
  - 9.2 Status "Lampu Mati Sebagian" (Ada Bohlam yang Putus)
  - 9.3 Status "Jalur Instalasi Putus" (Lampu Mati Total)
  - 9.4 Status "Tegangan Drop" atau "Tegangan Lebih"
  - 9.5 Status "Kebocoran Relay" (Lampu Tidak Mau Padam)
  - 9.6 Layar Monitor Mati atau Jam Menampilkan `--:--:--`
- [BAB 10: PERAWATAN RUTIN AGAR ALAT AWET BERTAHUN-TAHUN](#bab-10-perawatan-rutin-agar-alat-awet-bertahun-tahun)
  - 10.1 Jadwal Perawatan Harian, Bulanan, & Musiman
  - 10.2 Uji Ketahanan Baterai Cadangan UPS
- [BAB 11: TANYA JAWAB UMUM (FAQ)](#bab-11-tanya-jawab-umum-faq)

---

## BAB 1: MENGENAL SISTEM ENERGA & MANFAATNYA

### 1.1 Mengapa Kebun Buah Naga Membutuhkan Energa?

Tanaman buah naga (*dragon fruit*) secara alami hanya berbunga pada musim tertentu saat siang hari panjang. Untuk memicu pembungaan lebat di luar musim (*off-season*), kebun membutuhkan pencahayaan lampu buatan pada malam hari selama 4 sampai 6 jam (biasanya mulai pukul 18.00 hingga 22.00 atau 00.00).

Penyinaran manual konvensional memiliki banyak kendala:
* Petani harus datang ke kebun larut malam hanya untuk menyalakan dan mematikan sakelar.
* Jika ada beberapa bohlam yang putus atau fitting berkarat di tiang kebun yang luas, petani sering kali tidak menyadarinya sehingga induksi bunga gagal merata.
* Jika listrik PLN kebun padam di tengah malam, petani tidak tahu dan waktu penyinaran terbuang sia-sia.
* Risiko tagihan listrik bengkak akibat lampu lupa dimatikan atau kontaktor lengket.

**Sistem Energa** diciptakan untuk menjawab seluruh kendala tersebut dengan otomasi cerdas berbasis Internet (*IoT*).

---

### 1.2 Keuntungan Utama bagi Pemilik Kebun

```
  ┌────────────────────────────────────────────────────────────────────────┐
  │                        MANFAAT UTAMA ENERGA                            │
  ├────────────────────────────────────────────────────────────────────────┤
  │  1. KONTROL DARI RUMAH                                                 │
  │     Nyalakan atau matikan lampu kebun langsung dari layar HP Anda.     │
  │                                                                        │
  │  2. JADWAL OTOMATIS TEPAT WAKTU                                        │
  │     Lampu menyala dan padam sendiri sesuai timer, meski HP Anda mati.  │
  │                                                                        │
  │  3. DETEKSI OTOMATIS LAMPU PUTUS                                       │
  │     Alat langsung menghitung berapa bohlam yang mati di tiang kebun.   │
  │                                                                        │
  │  4. PERINGATAN LISTRIK PADAM                                           │
  │     Buzzer berbunyi dan HP Anda menerima notifikasi saat PLN padam.    │
  │                                                                        │
  │  5. PANTAU BIAYA LISTRIK REAL-TIME                                     │
  │     Ketahui pemakaian Watt dan total kWh untuk memprediksi tagihan.    │
  └────────────────────────────────────────────────────────────────────────┘
```

---

### 1.3 Pembagian Dua Blok Lampu (Blok A & Blok B)

Instalasi lampu kebun Anda dibagi menjadi dua kelompok terpisah:
* **Blok A:** Kelompok barisan tiang lampu bagian pertama (misalnya sisi Barat atau Blok Depan).
* **Blok B:** Kelompok barisan tiang lampu bagian kedua (misalnya sisi Timur atau Blok Belakang).

Kedua blok ini dapat dinyalakan sendiri-sendiri atau bersamaan melalui aplikasi di smartphone Anda.

---

### 1.4 Keamanan Daya Ganda (Listrik PLN & Baterai Cadangan UPS)

Sistem Energa menggunakan teknologi **Pemisahan Jalur Daya**:
1. **Lampu Kebun:** Menggunakan listrik langsung dari PLN 220V karena kebutuhan watt-nya besar. Jika PLN padam, lampu kebun ikut padam.
2. **Otak Kontrol & Internet:** Diberi daya cadangan dari **Baterai UPS**. Ketika listrik PLN kebun mati total, otak kontrol, layar monitor, alarm buzzer, dan modem internet **TETAP HIDUP**. Dengan demikian, alat sempat membunyikan alarm di kebun dan mengirimkan pesan peringatan bahaya ke HP Anda!

---

## BAB 2: MENGENAL BAGIAN-BAGIAN BOX PANEL DI KEBUN

### 2.1 Tampak Luar Box Panel

Alat Energa ditempatkan di dalam kotak panel pelindung berbahan plat logam tebal berstandar luar ruangan (tahan debu dan percikan air hujan).

* **Pintu Panel Berjendela Kaca:** Terdapat jendela transparan di pintu agar Anda dapat melihat tampilan layar monitor kecil di dalam tanpa perlu membuka pintu panel.
* **Kunci Pengaman Pintu:** Menggunakan kunci putar agar pintu selalu tertutup rapat dan aman dari jangkauan anak-anak atau hewan kebun.
* **Lubang Kabel Bawah (Cable Gland):** Seluruh kabel masuk dan keluar berada di bagian bawah box agar air hujan yang mengalir di kabel tidak masuk ke dalam panel.

---

### 2.2 Bagian Dalam Panel yang Perlu Diketahui Pengguna

Saat pintu panel dibuka (pastikan membaca petunjuk keselamatan terlebih dahulu), Anda akan melihat komponen-komponen berikut:

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│                             BAGIAN DALAM PANEL ENERGA                            │
│                                                                                  │
│   ┌──────────────┐          ┌───────────────────────────────────────────────┐    │
│   │ POWER SUPPLY │          │               PAPAN KONTROL ELEKTRONIK        │    │
│   │ LOGAM KECIL  │          │                                               │    │
│   │              │          │  [Trafo Mini]    [Komputer ESP32]  [Layar     │    │
│   │ (Pengubah    │          │  Deteksi PLN     Otak Sistem       Monitor    │    │
│   │  ke DC 12V)  │          │                                    OLED 0.96"]│    │
│   └──────────────┘          └───────────────────────────────────────────────┘    │
│                                                                                  │
│   ────────────────────────────────────────────────────────────────────────────   │
│    [Stop Kontak]     [Terminal Kabel Abu-Abu]       [Kontaktor Saklar Besar]     │
│    Colokan UPS       Jalur Sambungan Kabel Kebun     (TOMZN Blok A & Blok B)     │
│   ────────────────────────────────────────────────────────────────────────────   │
│                                                                                  │
│   ┌──────────────────────────────────────────────────────────────────────────┐   │
│   │                         UNIT BATERAI CADANGAN (UPS)                      │   │
│   │                 (Menjaga alat tetap hidup saat PLN mati)                 │   │
│   └──────────────────────────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────────────────────────┘
```

1. **Layar Monitor OLED (Ukuran 0.96 Inci):** Layar kecil di papan atas yang menampilkan jam kebun, kondisi WiFi, daya watt, dan kondisi lampu.
2. **Kontaktor Saklar Besar (TOMZN):** Saklar magnetik berdaya tinggi. Saat lampu dinyalakan dari HP, kontaktor ini akan berbunyi *"Cklek"* tegas sebagai tanda aliran listrik ke ratusan lampu kebun telah terhubung.
3. **Stop Kontak DIN Rail:** Tempat mencolokkan kabel power supply kontrol ke sumber baterai cadangan.
4. **Terminal Kabel Abu-Abu:** Tempat tersambungnya kabel dari meteran PLN dan kabel distribusi yang menuju tiang lampu kebun.

---

### 2.3 Perangkat Tambahan (Modem 4G & Baterai UPS)

* **Unit Cadangan Daya (UPS):** Baterai kotak hitam yang diletakkan di bagian bawah panel atau di dekat panel. Memiliki tombol ON/OFF dan lampu indikator baterai.
* **Modem WiFi 4G LTE:** Perangkat pemancar internet kebun (menggunakan kartu SIM seluler) yang dicolokkan ke sumber listrik UPS agar selalu terhubung ke aplikasi smartphone Anda.

---

## BAB 3: PANDUAN KESELAMATAN KERJA (K3) PETANI

> [!CAUTION]
> **BAHAYA TEGANGAN TINGGI 220V AC:**  
> Di dalam box panel mengalir arus listrik bertegangan tinggi yang dapat menyebabkan sengatan fatal. Patuhi selalu aturan keselamatan di bawah ini!

### 3.1 Aturan Penting Kelistrikan Kebun

1. **Gunakan Alas Kaki Karet Kering:** Tanah perkebunan buah naga sering basah dan becek. Jangan pernah membuka pintu panel atau menyentuh bodi panel dengan bertelanjang kaki atau menggunakan sandal basah.
2. **Dilarang Membuka Panel Saat Hujan:** Jangan membuka pintu box panel saat sedang hujan lebat, gerimis berangin, atau saat mesin semprot air/sprinkler kebun sedang menyala.
3. **Kunci Selalu Pintu Box Panel:** Setelah selesai mengecek, pastikan pintu panel ditutup rapat dan dikunci agar serangga malam (seperti kecoak, semut, atau cicak) serta percikan air tidak masuk ke rangkaian elektronik.

---

### 3.2 Peringatan Khusus: Baterai Cadangan UPS Tetap Menyala!

> [!WARNING]
> **PERINGATAN SANGAT PENTING: TEGANGAN TETAP ADA SAAT PLN PADAM!**  
> Panel ini memiliki baterai cadangan (UPS). Meskipun listrik PLN kebun sedang padam atau MCB PLN Anda turunkan, **bagian stop kontak dan kabel papan kontrol di dalam panel MASIH BERTEGANGAN 220V!**

**Cara Mematikan Aliran Listrik Secara Total (Zero-Energy):**  
Jika Anda atau teknisi perlu menyentuh kabel di dalam panel:
1. Turunkan tuas MCB PLN kebun ke posisi **OFF**.
2. Tekan dan tahan tombol Power pada unit UPS hingga lampu indikatornya padam dan bunyi *bip* berhenti.
3. Cabut steker kabel hitam dari stop kontak Schneider di dalam panel.
4. Uji kabel dengan tespen sebelum menyentuh kabel apa pun.

---

### 3.3 Tindakan Cepat Saat Terjadi Kondisi Darurat

Jika Anda melihat atau mencium hal-hal berikut dari dalam panel:
* Tercium bau plastik atau kabel terbakar / sangit.
* Muncul asap atau percikan bunga api listrik.
* Terdengar suara dengung yang sangat keras dan bergetar hebat.

**Langkah Penyelamatan:**
1. **JANGAN MENYENTUH BODI LOGAM PANEL DENGAN TANGAN TELANJANG!**
2. Segera matikan aliran listrik dari MCB induk yang berada di tiang luar atau meteran PLN utama.
3. Jika aman dijangkau, matikan saklar UPS.
4. Gunakan Alat Pemadam Api Ringan (APAR) serbuk atau gas CO2 jika terjadi api. **Dilarang keras menyiram panel dengan air!**
5. Segera hubungi teknisi penanggung jawab kebun.

---

## BAB 4: CARA MENYALAKAN & MEMATIKAN ALAT (SOP HARIAN)

### 4.1 Prosedur Menyalakan Alat Pertama Kali

Ikuti urutan langkah berikut saat pertama kali menyalakan panel:

```
Langkah 1:                      Langkah 2:                      Langkah 3:
Nyalakan Baterai UPS            Nyalakan MCB PLN Utama          Periksa Layar Monitor
┌────────────────────────┐      ┌────────────────────────┐      ┌────────────────────────┐
│ Tekan tombol Power UPS │      │ Naikkan tuas MCB PLN   │      │ Layar OLED menyala dan │
│ sampai bunyi "Bip" 1x  │ ───► │ ke posisi ATAS (ON)    │ ───► │ berbunyi "Tit" pendek. │
│ Indikator hijau nyala. │      │ Aliran AC 220V masuk.  │      │ Muncul tulisan jam.    │
└────────────────────────┘      └────────────────────────┘      └────────────────────────┘
```

1. **Langkah 1:** Tekan tombol Power pada unit UPS hingga lampu indikator baterai menyala hijau dan terdengar bunyi *bip* satu kali.
2. **Langkah 2:** Naikkan tuas sakelar MCB PLN utama ke posisi **ON** (ke atas).
3. **Langkah 3:** Perhatikan layar monitor kecil di dalam panel.

---

### 4.2 Tanda Alat Sudah Siap Beroperasi Normal

Alat sudah bekerja sempurna jika:
1. Buzzer mengeluarkan bunyi **"Tit" pendek (1 kali)**.
2. Layar monitor menampilkan tulisan pembuka, lalu berganti menampilkan jam kebun yang berkedip normal.
3. Pada baris ketiga tertulis: **`Source: PLN`** (artinya listrik utama aktif).
4. Pada baris kedua tertulis: **`Wifi: online`** (artinya alat sudah terhubung ke internet dan HP Anda).

---

### 4.3 Prosedur Mematikan Panel Secara Aman (Shutdown)

Jika musim panen telah selesai dan lampu tidak akan digunakan selama berminggu-minggu:
1. Pastikan kedua blok lampu dalam posisi padam (OFF) melalui aplikasi smartphone.
2. Turunkan tuas MCB PLN ke posisi **OFF** (ke bawah).
3. Matikan unit UPS dengan menekan tombol Power-nya hingga seluruh lampu indikator padam.

---

## BAB 5: CARA MENGHUBUNGKAN ALAT KE WIFI / MODEM KEBUN LEWAT HP

### 5.1 Kapan Perlu Mengatur WiFi?

Anda hanya perlu melakukan pengaturan WiFi ini jika:
* Panel baru pertama kali dipasang di kebun.
* Anda mengganti modem WiFi kebun baru atau mengganti nama/password WiFi.
* Layar monitor di panel menampilkan tulisan:
  ```
  == SETUP WIFI ==
  1. Konek ke WiFi:
     Energa-AP
  2. Buka 192.168.4.1
  ```

---

### 5.2 Langkah Cepat Sambung WiFi (Hanya 3 Menit)

Pegang smartphone Anda, berdiri di dekat panel, lalu ikuti 4 langkah mudah berikut:

```
[LANGKAH 1]                [LANGKAH 2]                [LANGKAH 3]                [LANGKAH 4]
Buka Menu Wi-Fi di HP      Buka Web Browser           Pilih Nama WiFi Kebun      Klik Simpan (Save)
┌────────────────────┐     ┌────────────────────┐     ┌────────────────────┐     ┌────────────────────┐
│ Pengaturan Wi-Fi   │     │ Di kolom website:  │     │   Energa Portal    │     │   Energa Portal    │
│                    │     │ ketik:             │     │                    │     │ SSID: Modem_Kebun  │
│ ✓ Energa-AP        │ ──► │ 192.168.4.1        │ ──► │ [Configure WiFi]   │ ──► │ Pass: [kebun1234 ] │
│   Modem_Kebun      │     │ lalu tekan Masuk   │     │                    │     │                    │
│   (Tanpa Password) │     │                    │     │ Pilih: Modem_Kebun │     │ [   S A V E   ]    │
└────────────────────┘     └────────────────────┘     └────────────────────┘     └────────────────────┘
```

1. **Sambungkan HP ke WiFi Panel:**
   * Buka menu **Pengaturan Wi-Fi** di HP Anda.
   * Pilih jaringan bernama **`Energa-AP`**. Jaringan ini terbuka (*Open*), jadi langsung tersambung tanpa meminta kata sandi.
2. **Buka Halaman Pengaturan di HP:**
   * Biasanya layar HP akan otomatis menampilkan jendela baru.
   * Jika tidak muncul otomatis, buka aplikasi **Google Chrome** atau **Safari** di HP, ketik alamat: **`192.168.4.1`** lalu tekan Enter.
3. **Pilih WiFi Kebun Anda:**
   * Pada halaman web yang terbuka, klik tombol **Configure WiFi**.
   * HP akan menampilkan daftar sinyal WiFi di kebun. Klik nama WiFi modem kebun Anda.
   * Ketikkan kata sandi WiFi modem Anda dengan benar di kolom password.
4. **Simpan:**
   * Tekan tombol **Save**. Panel akan menyimpan data tersebut dan menyambung ke internet secara otomatis.

---

### 5.3 Cara Memastikan Koneksi Sudah Berhasil

Lihat baris kedua pada layar monitor kecil di box panel:
* Tertulis **`Wifi: online 45ms`** ➔ **Sempurna!** Alat sudah terhubung ke internet dan siap dipantau dari aplikasi HP.
* Tertulis **`Wifi: No Internet`** ➔ Alat sudah terhubung ke modem, tetapi kartu kuota internet modem habis atau sinyal seluler sedang hilang.
* Tertulis **`Wifi: offline`** ➔ Modem kebun sedang mati atau jangkauan sinyal WiFi terlalu jauh dari panel.

---

## BAB 6: CARA MEMBACA LAYAR MONITOR DI BOX PANEL

Anda tidak perlu selalu membuka HP untuk melihat kondisi kebun. Cukup intip melalui kaca pintu panel untuk membaca layar monitor 7 baris:

### 6.1 Susunan Tampilan 7 Baris di Layar

```text
┌──────────────────────────────┐
│           18:30:15           │ ◄── Baris 1: Jam Saat Ini di Kebun (WIB)
│ Wifi: online 45ms            │ ◄── Baris 2: Status Sambungan Internet & Kecepatan
│ Source: PLN                  │ ◄── Baris 3: Sumber Listrik (PLN atau UPS)
│ Blok A: 1250W                │ ◄── Baris 4: Daya Listrik Blok A yang Dipakai
│ Status: 10/10 Normal         │ ◄── Baris 5: Kondisi Kesehatan Lampu Blok A
│ Blok B: 1240W                │ ◄── Baris 6: Daya Listrik Blok B yang Dipakai
│ Status: 10/10 Normal         │ ◄── Baris 7: Kondisi Kesehatan Lampu Blok B
└──────────────────────────────┘
```

---

### 6.2 Memahami Arti Informasi di Layar

1. **Baris 1 (Jam):** Jam waktu nyata kebun. Jam ini selalu tepat karena menyelaraskan otomatis dengan satelit internet dan memiliki baterai jam mandiri.
2. **Baris 2 (WiFi):** Menunjukkan apakah alat bisa berkomunikasi dengan HP Anda lewat internet.
3. **Baris 3 (Source):**
   * `Source: PLN` = Listrik utama kebun menyala normal.
   * `Source: UPS` = **Listrik PLN mati!** Panel saat ini sedang hidup menggunakan baterai cadangan.
4. **Baris 4 & 6 (Daya Blok A & B):** Menunjukkan berapa Watt listrik yang sedang disedot oleh lampu di blok tersebut.
5. **Baris 5 & 7 (Status Lampu):** Menunjukkan apakah lampu menyala normal atau ada yang putus.

---

### 6.3 Contoh Tampilan Saat Semua Lampu Sehat

Ketika lampu dinyalakan dan semua bohlam berfungsi baik:
```text
Blok A: 1250W
Status: 10/10 Normal
```
*Artinya:* Dari total 10 lampu yang dipasang di Blok A, **semua 10 lampu menyala normal** dengan daya total 1250 Watt.

Ketika siang hari (lampu sedang dimatikan):
```text
Blok A: 0W
Status: Standby
```
*Artinya:* Lampu sedang padam/istirahat normal, tidak ada kebocoran listrik.

---

### 6.4 Contoh Tampilan Saat Ada Lampu yang Putus

Jika tertiup angin atau ada bohlam yang putus/mati:
```text
Blok A: 1000W
Status: 8/10 Normal
```
*Artinya:* Dari 10 lampu yang seharusnya menyala, alat mendeteksi hanya **8 lampu yang hidup** (ada **2 lampu yang padam**). Anda bisa segera menyiapkan 2 bohlam pengganti.

---

## BAB 7: ARTI BUNYI ALARM (BUZZER) & TINDAKAN ANDA

Alat Energa dilengkapi alarm suara agar petugas di kebun langsung mengetahui jika terjadi sesuatu:

| Pola Suara Alarm | Kapan Terjadi? | Apa Artinya? | Tindakan yang Harus Dilakukan |
|---|---|---|---|
| **"Tit" Pendek (1x)**<br>*(0.2 detik)* | Saat tombol saklar di HP ditekan | **Perintah Diterima:** Kontaktor sukses menyalakan atau mematikan lampu kebun. | Tidak ada, sistem bekerja normal. |
| **"Tiiit ... Tiiit ..."**<br>*(Bip terputus-putus tiap 2 detik)* | Saat listrik PLN kebun padam | **PERINGATAN LISTRIK PADAM:** Aliran listrik PLN mati. Lampu kebun padam, tetapi panel tetap hidup berkat baterai UPS. | Periksa apakah listrik satu desa padam atau MCB PLN kebun turun. Nyalakan genset jika ada jadwal penyinaran wajib. |
| **"Tit-tit-tit-tit!"**<br>*(Bip sangat cepat rapat)* | Terjadi lonjakan daya abnormal | **BAHAYA BEBAN BERLEBIH:** Terjadi korsleting kabel atau ada beban liar yang disambungkan. | Segera matikan lampu dari HP atau turunkan MCB beban. Periksa kabel di kebun. |
| **Senyap (Tanpa Bunyi)** | Operasional sehari-hari | Sistem berjalan aman dan normal. | - |

---

## BAB 8: PANDUAN MENGGUNAKAN APLIKASI DI SMARTPHONE (ENERGA APP)

Semua kendali kebun berada di genggaman Anda melalui aplikasi **Energa** di smartphone Android atau iPhone.

### 8.1 Membuka Aplikasi & Melihat Status Kebun Real-Time

1. Buka aplikasi **Energa** di smartphone Anda.
2. Di halaman utama (**Dashboard**), Anda akan melihat:
   * **Status Alat:** Tertulis *Online* (titik hijau) jika panel kebun terhubung ke internet.
   * **Kartu Blok A & Blok B:** Menampilkan saklar lampu, daya Watt saat ini, dan status kesehatan lampu.
   * **Tegangan Listrik (Volt):** Menginfokan voltase listrik kebun (normalnya 210V – 230V).
   * **Total Energi (kWh):** Menunjukkan berapa kilowatt-jam listrik yang telah digunakan.

---

### 8.2 Menyalakan & Mematikan Lampu Manual Lewat HP

Jika sewaktu-waktu Anda ingin menyalakan lampu kebun di luar jadwal rutin (misalnya saat inspeksi malam atau penyemprotan pupuk):
1. Cukup sentuh **Tombol Saklar** pada kartu Blok A atau Blok B di aplikasi.
2. Dalam waktu kurang dari 1 detik, kontaktor di panel kebun akan berbunyi *"Cklek"*, alat berbunyi *"Tit"*, dan lampu di kebun langsung menyala.
3. Untuk mematikan kembali, sentuh tombol saklar tersebut ke posisi **OFF**.

---

### 8.3 Mengatur Jadwal Lampu Otomatis (Timer Penyinaran Malam)

Anda tidak perlu begadang setiap malam untuk mengontrol lampu. Gunakan fitur **Jadwal Otomatis (Scheduler)**:

```
┌────────────────────────────────────────────────────────┐
│               PENGATURAN JADWAL LAMPU                  │
│                                                        │
│  [✓] Aktifkan Jadwal Otomatis                          │
│                                                        │
│  Waktu Nyala : [ 18 : 00 ] Sore                        │
│  Waktu Padam : [ 22 : 00 ] Malam                       │
│                                                        │
│  Hari Berulang:                                        │
│  [Sen]  [Sel]  [Rab]  [Kam]  [Jum]  [Sab]  [Min]       │
│                                                        │
│  [  SIMPAN JADWAL KE PERANGKAT  ]                      │
└────────────────────────────────────────────────────────┘
```

1. Buka menu **Jadwal (Schedule)** di aplikasi Energa.
2. Tentukan **Waktu Nyala** (contoh: pukul `18:00`).
3. Tentukan **Waktu Padam** (contoh: pukul `22:00` atau `00:00`).
4. Centang hari-hari penyinaran yang diinginkan.
5. Klik **Simpan Jadwal**.

> [!NOTE]
> **Kelebihan Timer Energa:**  
> Jadwal yang Anda simpan akan dikirim dan dicatat langsung ke dalam chip memori jam mandiri di box panel. Jadi, **meskipun sinyal internet modem kebun hilang atau HP Anda mati kehabisan baterai**, lampu kebun akan tetap menyala dan padam sendiri secara tepat waktu setiap malam!

---

### 8.4 Mengisi Data Konfigurasi Lampu (PENTING untuk Deteksi Kerusakan)

Agar alat bisa menghitung secara tepat jika ada bohlam lampu yang putus, Anda wajib mengisi data lampu kebun Anda pada menu pengaturan:

1. Buka menu **Pengaturan** ➔ pilih **Konfigurasi Beban Kebun**.
2. Masukkan 3 data sederhana berikut untuk Blok A dan Blok B:
   * **Jumlah Lampu (`lamp_count`):** Berapa total bohlam lampu yang terpasang di blok tersebut (contoh: `100` buah).
   * **Daya Total Normal (`expected_power`):** Total watt seluruh lampu saat menyala normal (contoh: jika memakai 100 lampu masing-masing 15 Watt, isi: `1500` Watt).
   * **Toleransi (`tolerance_percent`):** Batas toleransi fluktuasi voltase desa (biarkan pada angka rekomendasi `15%`).
3. Klik tombol **Simpan Konfigurasi**.

*Kapan data ini perlu diubah?* Hanya jika Anda menambah tiang buah naga baru atau mengganti ukuran watt bohlam lampu Anda.

---

### 8.5 Memantau Pemakaian Energi & Perkiraan Tagihan Listrik

* Buka menu **Laporan Energi (Energy History)** di aplikasi.
* Anda dapat melihat grafik pemakaian listrik harian, mingguan, hingga bulanan (dalam kWh).
* Anda dapat memperkirakan biaya token/tagihan listrik PLN kebun sebelum tagihan resmi keluar, sehingga biaya operasional kebun lebih terkendali.

---

## BAB 9: PANDUAN MENGATASI GANGGUAN DI LAPANGAN (TROUBLESHOOTING)

Jika muncul tulisan peringatan pada layar monitor panel atau notifikasi darurat di HP, gunakan tabel panduan praktis berikut:

### 9.1 Status "Listrik Padam" (PLN Mati)
* **Penyebab:** Aliran listrik PLN dari gardu desa terputus saat jadwal lampu menyala.
* **Tanda di Kebun:** Lampu kebun padam, buzzer berbunyi terputus-putus, layar menampilkan `Source: UPS` dan `Status: Listrik Padam`.
* **Tindakan Anda:** 
  1. Periksa apakah tetangga sekitar kebun juga mengalami mati listrik.
  2. Anda tidak perlu mematikan alat secara manual. Begitu listrik PLN kembali menyala, alat akan otomatis mendeteksi dan menyalakan lampu kembali sesuai jadwal.
  3. Jika jadwal penyinaran malam ini sangat krusial, nyalakan genset kebun untuk menggantikan sumber PLN.

---

### 9.2 Status "Lampu Mati Sebagian" (Ada Bohlam yang Putus)
* **Penyebab:** Beberapa bohlam lampu LED di tiang kebun putus, pecah tertiup angin, atau fitting lampu berkarat.
* **Tanda di Kebun:** Layar monitor menuliskan perbandingan jumlah lampu, contohnya: `Status: 18/20 Normal`.
* **Tindakan Anda:** 
  1. Angka `18/20` artinya ada **2 bohlam yang mati**.
  2. Bawa 2 bohlam cadangan, jalan menyusuri deretan tiang buah naga di blok tersebut pada malam hari.
  3. Cari 2 bohlam yang tidak berpijar, lalu ganti dengan bohlam baru.
  4. Begitu lampu baru terpasang, angka di layar akan otomatis berubah kembali menjadi `20/20 Normal`.

---

### 9.3 Status "Jalur Instalasi Putus" (Lampu Mati Total)
* **Penyebab:** Saklar lampu dalam posisi ON dan listrik PLN hidup, tetapi tidak ada daya sama sekali (0 Watt). Biasanya karena tuas MCB beban turun (*trip*) akibat korsleting, atau kabel fasa utama menuju tiang putus tertimpa dahan pohon.
* **Tindakan Anda:**
  1. Buka box panel, periksa tuas MCB beban lampu. Jika tuas turun ke bawah, naikkan kembali ke atas.
  2. Jika tuas MCB langsung mental turun kembali dengan bunyi letupan kecil, **JANGAN DIPAKSAKAN!** Pasti ada kabel tiang yang terkelupas atau bersentuhan dengan dahan basah. Periksa instalasi kabel di kebun terlebih dahulu.

---

### 9.4 Status "Tegangan Drop" atau "Tegangan Lebih"
* **Penyebab:** Tegangan jala-jala listrik PLN desa sedang tidak stabil (turun di bawah 190 Volt atau melonjak di atas 245 Volt).
* **Tindakan Anda:** 
  * Tegangan drop sering terjadi saat jam beban puncak desa (maghrib). Jika kondisi ini terjadi terus-menerus dan lampu tampak redup/berkedip, laporkan ke kantor PLN terdekat untuk perbaikan trafo distribusi di jalur pertanian Anda.

---

### 9.5 Status "Kebocoran Relay" (Lampu Tidak Mau Padam)
* **Penyebab:** Anda sudah mematikan lampu di aplikasi HP (posisi OFF), tetapi lampu di tiang kebun tetap menyala atau terdeteksi arus mengalir. Hal ini terjadi jika kontak saklar kontaktor lengket.
* **Tindakan Anda:**
  1. Segera turunkan tuas MCB lampu di dalam panel untuk mematikan lampu secara paksa agar listrik tidak terbuang percuma di siang hari.
  2. Hubungi teknisi untuk memeriksa dan mengetuk/mengganti unit kontaktor magnetik.

---

### 9.6 Layar Monitor Mati atau Jam Menampilkan `--:--:--`
* **Jika Layar Gelap Gulita:** Periksa tombol power baterai cadangan (UPS). Jika baterai UPS habis total, hidupkan sumber PLN untuk mengisi ulang baterai UPS.
* **Jika Jam Menampilkan `--:--:--`:** Baterai kancing bulat (CR2032) di dalam papan kontrol telah habis. Ganti baterai kancing tersebut dengan yang baru (tersedia di toko jam/elektronik), lalu hubungkan alat ke WiFi agar jam kembali tersinkronisasi via internet.

---

## BAB 10: PERAWATAN RUTIN AGAR ALAT AWET BERTAHUN-TAHUN

### 10.1 Jadwal Perawatan Harian, Bulanan, & Musiman

Untuk menjaga performa panel Energa agar beroperasi stabil hingga bertahun-tahun di lingkungan kebun yang berdebu dan lembap, ikuti jadwal perawatan berikut:

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│                           JADWAL PERAWATAN ALAT ENERGA                           │
├──────────────────────────────────────────────────────────────────────────────────┤
│ • SETIAP HARI (Cukup 10 Detik)                                                   │
│   Intip jendela kaca panel saat melintas: Pastikan jam berkedip normal dan baris │
│   WiFi tertulis "online".                                                        │
│                                                                                  │
│ • SETIAP BULAN (Pembersihan Ringan)                                              │
│   Buka pintu panel di siang hari yang kering. Periksa apakah ada sarang semut    │
│   atau serangga malam di sudut panel. Bersihkan debu luar dengan kuas kering.    │
│                                                                                  │
│ • SETIAP 6 BULAN (Pengecekan Baut & Baterai)                                     │
│   1. Minta teknisi listrik memeriksa kekencangan baut terminal kabel.            │
│   2. Lakukan uji coba ketahanan baterai cadangan UPS (lihat langkah di bawah).   │
│                                                                                  │
│ • SETIAP 2 TAHUN                                                                 │
│   Ganti baterai kancing bulat jam (CR2032) di papan PCB kontrol.                 │
└──────────────────────────────────────────────────────────────────────────────────┘
```

---

### 10.2 Uji Ketahanan Baterai Cadangan UPS

Lakukan pengetesan baterai UPS setiap 3 atau 6 bulan sekali di siang hari:
1. Pastikan baterai UPS sudah terisi penuh sebelumnya.
2. Turunkan tuas MCB PLN utama untuk mensimulasikan pemadaman listrik.
3. Layar monitor harus tetap menyala normal, baris ketiga berubah menjadi `Source: UPS`, dan buzzer membunyikan alarm bip terputus-putus.
4. Biarkan selama 10 hingga 15 menit. Jika layar tetap menyala stabil dan modem internet tetap aktif, baterai cadangan UPS Anda masih dalam kondisi sangat sehat!
5. Naikkan kembali MCB PLN utama ke atas.

---

## BAB 11: TANYA JAWAB UMUM (FAQ)

**T: Jika sinyal modem internet di kebun hilang atau paket data habis, apakah jadwal lampu malam tetap menyala otomatis?**  
**J:** **YA, TETAP MENYALA 100%!** Sistem Energa memiliki jam mandiri internal yang terus berdetik di dalam panel. Walaupun koneksi internet mati berhari-hari, lampu kebun Anda tetap akan menyala dan padam sendiri secara otomatis sesuai jadwal yang sudah tersimpan.

**T: Apakah alat ini aman dari semprotan air hujan lebat dan petir?**  
**J:** Box panel menggunakan standar *outdoor IP54/IP65* dengan lapisan karet kedap air pada pintunya. Namun, untuk melindungi kebun dari sambaran petir di area terbuka, pastikan tiang panel telah dipasangi kawat pembumian tembaga (*grounding*) yang tertancap ke tanah kebun sedalam minimal 1.5 hingga 2 meter.

**T: Saya baru saja mengganti modem WiFi kebun baru, bagaimana cara menyambungkannya kembali?**  
**J:** Cukup ikuti panduan praktis di **BAB 5**. Buka menu WiFi di HP, sambungkan ke `Energa-AP`, buka alamat `192.168.4.1` di browser HP Anda, lalu pilih nama WiFi modem baru Anda dan masukkan sandinya.

**T: Bisakah aplikasi Energa dipasang di beberapa HP sekaligus?**  
**J:** Sangat bisa. Aplikasi dapat dipasang di smartphone pemilik kebun, mandor, dan penjaga malam dengan masuk (*login*) menggunakan akun kebun yang sama, sehingga pemantauan dapat dilakukan bersama-sama.

---

```
                       AKHIR DOKUMEN MANUAL BOOK
         Energa Smart Farming System — Solusi Cerdas Buah Naga Unggul
```

