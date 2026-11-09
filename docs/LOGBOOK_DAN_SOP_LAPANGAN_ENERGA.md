# LAMPIRAN OPERASIONAL & LOGBOOK LAPANGAN
## Energa — Sistem Kontrol & Monitoring Pencahayaan Buah Naga

Dokumen ini merupakan lampiran praktis pelengkap dari **[MANUAL_BOOK_OPSI_2.md](MANUAL_BOOK_OPSI_2.md)**. Berisi lembar siap cetak untuk operasional harian petani, rekomendasi jadwal penyinaran agronomi, dan formulir pencatatan pemeliharaan di kebun buah naga.

---

## DAFTAR ISI LAMPIRAN

1. [LAMPIRAN 1: LEMBAR SOP RINGKAS (UNTUK DITEMPEL DI PINTU PANEL)](#lampiran-1-lembar-sop-ringkas-untuk-ditempel-di-pintu-panel)
2. [LAMPIRAN 2: PEDOMAN AGRONOMI PENYINARAN BUAH NAGA OFF-SEASON](#lampiran-2-pedoman-agronomi-penyinaran-buah-naga-off-season)
3. [LAMPIRAN 3: FORMULIR LOGBOOK PENYINARAN & PENGGANTIAN BOHLAM](#lampiran-3-formulir-logbook-penyinaran--penggantian-bohlam)
4. [LAMPIRAN 4: FORMULIR LOGBOOK PEMELIHARAAN ALAT & BATERAI UPS](#lampiran-4-formulir-logbook-pemeliharaan-alat--baterai-ups)

---

## LAMPIRAN 1: LEMBAR SOP RINGKAS (UNTUK DITEMPEL DI PINTU PANEL)

> *Cetak halaman ini pada kertas A4, masukkan ke dalam plastik laminating, lalu tempelkan pada bagian dalam pintu box panel kebun.*

```text
==================================================================================
                 PETUNJUK CEPAT PENGOPERASIAN PANEL ENERGA
==================================================================================

1. CARA MENYALAKAN ALAT (SORE HARI / PERTAMA KALI)
   [1] Tekan tombol Power pada Baterai UPS hingga lampu indikator menyala hijau.
   [2] Naikkan tuas MCB PLN Utama ke posisi ATAS (ON).
   [3] Tunggu bunyi "Tit" 1x dan layar monitor OLED menampilkan jam kebun.
   [4] Pastikan baris ke-2 tertulis "Wifi: online" dan baris ke-3 "Source: PLN".

2. CARA SAMBUNG KE WIFI / MODEM BARU (3 MENIT)
   Jika layar menampilkan tulisan "== SETUP WIFI ==":
   [1] Buka WiFi di HP -> Sambungkan ke "Energa-AP" (Tanpa Password).
   [2] Buka browser (Chrome/Safari) di HP -> Ketik: 192.168.4.1
   [3] Klik "Configure WiFi" -> Pilih WiFi kebun Anda -> Masukkan kata sandi.
   [4] Klik "Save". Panel akan restart sendiri hingga layar tertulis "Wifi: online".

3. CARA MEMBACA LAYAR MONITOR KECIL
   ┌──────────────────────────────┐
   │           18:30:00           │ ──> Jam Kebun (Otomatis & Tepat)
   │ Wifi: online 45ms            │ ──> Internet Lancar (Terkoneksi ke HP)
   │ Source: PLN                  │ ──> Listrik Utama PLN Normal
   │ Blok A: 1250W                │ ──> Pemakaian Listrik Lampu Blok A
   │ Status: 10/10 Normal         │ ──> Semua Lampu Blok A Sehat
   │ Blok B: 1240W                │ ──> Pemakaian Listrik Lampu Blok B
   │ Status: 10/10 Normal         │ ──> Semua Lampu Blok B Sehat
   └──────────────────────────────┘

4. ARTI SUARA ALARM (BUZZER)
   * Beep Pendek ("Tit" 1x)       : Saklar HP sukses menyalakan / mematikan lampu.
   * Beep Terputus ("Tiiit...")   : LISTRIK PLN MATI! Panel hidup dengan baterai UPS.
   * Beep Cepat ("Tit-tit-tit!")  : BAHAYA KORSLETING / Beban lebih di kebun.

5. PANDUAN CEPAT SAAT TERJADI GANGGUAN
   * Layar tulis "8/10 Normal"    : Ada 2 bohlam mati di kebun. Segera cari & ganti.
   * Layar tulis "Listrik Padam"  : PLN desa padam. Lampu mati, panel ditopang UPS.
   * Layar tulis "Jalur Putus"    : MCB beban trip atau kabel tiang tertimpa dahan.
   * Layar tulis "Wifi: No Net"   : Kuota modem habis. Lampu tetap nyala sesuai jadwal.
   * Lampu tidak mau mati saat OFF: Turunkan tuas MCB lampu, hubungi teknisi.

==================================================================================
  PERINGATAN K3: Gunakan alas kaki karet kering. Dilarang buka panel saat hujan.
==================================================================================
```

---

## LAMPIRAN 2: PEDOMAN AGRONOMI PENYINARAN BUAH NAGA OFF-SEASON

Untuk memaksimalkan pembungaan buah naga di luar musim (*off-season*), gunakan panduan teknis budidaya berikut:

### 2.1 Waktu & Durasi Penyinaran Ideal
* **Musim Penyinaran:** Dijalankan pada musim kemarau menjelang musim peralihan (biasanya bulan Maret–September di Indonesia) saat hari alami terasa pendek.
* **Jam Operasional Harian:** Mulai pukul **18.00 WIB s/d 22.00 WIB** (4 jam penyinaran tambahan) atau pukul **18.00 WIB s/d 00.00 WIB** (6 jam penyinaran tambahan).
* **Periode Siklus:** Lampu dinyalakan rutin setiap malam selama **15 sampai 21 hari berturut-turut** hingga kuncup bunga (*flower bud*) mulai muncul sebesar biji kedelai pada sulur tanaman.
* **Periode Istirahat:** Setelah kuncup bunga muncul merata, matikan siklus penyinaran lampu selama 10–14 hari untuk membiarkan bunga berkembang mekar alami dan penyerbukan terjadi tanpa gangguan cahaya.

### 2.2 Spesifikasi Lampu yang Direkomendasikan
* **Tipe Lampu:** Lampu LED Hemat Energi (Warm White / Cahaya Kuning Hangat, 2700K - 3000K) atau Lampu Spiral CFL (15 - 20 Watt).
* **Posisi Lampu:** Digantung tepat di tengah antara 4 tiang penyangga buah naga dengan ketinggian **1.5 hingga 1.8 meter di atas tanah** (cahaya merata mengenai sulur produktif bagian atas dan samping).

---

## LAMPIRAN 3: FORMULIR LOGBOOK PENYINARAN & PENGGANTIAN BOHLAM

> *Gunakan tabel berikut untuk mencatat operasional harian kebun per siklus induksi bunga:*

**Nama Kebun:** \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_  
**Musim / Periode:** \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_  
**Petugas / Penanggung Jawab:** \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_  

| No | Tanggal | Jam Nyala (WIB) | Jam Padam (WIB) | Pembacaan Daya Blok A (W) | Pembacaan Daya Blok B (W) | Jumlah Lampu Diganti (Buah) | Catatan Kondisi Lapangan (Hujan / PLN Mati / Bunga Muncul) | Tanda Tangan Mandor |
|:--:|:---:|:---:|:---:|:---:|:---:|:---:|:---|:---:|
| 1 | | | | | | | | |
| 2 | | | | | | | | |
| 3 | | | | | | | | |
| 4 | | | | | | | | |
| 5 | | | | | | | | |
| 6 | | | | | | | | |
| 7 | | | | | | | | |
| 8 | | | | | | | | |
| 9 | | | | | | | | |
| 10 | | | | | | | | |
| 11 | | | | | | | | |
| 12 | | | | | | | | |
| 13 | | | | | | | | |
| 14 | | | | | | | | |
| 15 | | | | | | | | |
| 16 | | | | | | | | |
| 17 | | | | | | | | |
| 18 | | | | | | | | |
| 19 | | | | | | | | |
| 20 | | | | | | | | |

---

## LAMPIRAN 4: FORMULIR LOGBOOK PEMELIHARAAN ALAT & BATERAI UPS

> *Lakukan pengecekan rutin berkala dan catat pada formulir berikut:*

| Tanggal Uji | Jenis Pengecekan | Prosedur yang Dilakukan | Hasil Pengecekan | Status (OK / Perlu Tindakan) | Petugas Pemeriksa |
|:---:|---|---|---|:---:|:---:|
| | **Uji Baterai UPS** | Matikan MCB PLN selama 10 menit di siang hari. | Layar tetap hidup / mati saat PLN dimatikan. Durasi tahan: \_\_\_ menit. | | |
| | **Kebersihan Panel** | Buka box panel, bersihkan debu, periksa sarang semut / serangga. | Kondisi dalam box bersih / kotor / lembap. | | |
| | **Kekencangan Baut** | Periksa baut terminal kabel AC dan kontaktor dengan obeng isolasi. | Seluruh baut kencang / ada baut kendor. | | |
| | **Kondisi Klem CT** | Pastikan klem sensor CT PZEM terpasang kokoh pada kabel fasa. | CT terpasang kokoh / kendor. | | |
| | **Baterai Kancing Jam** | Matikan total daya panel, hidupkan lagi. Pastikan jam RTC tidak reset. | Jam tetap tepat / reset ke `--:--:--`. | | |
| | **Karet Pintu Box** | Periksa elastisitas karet gasket pintu panel (pencegah air hujan). | Karet lentur rapat / getas pecah. | | |
| | **Grounding Rod** | Periksa kawat pembumian tembaga di tiang penyangga panel. | Kawat terhubung kuat / putus. | | |

---

```
                       DOKUMEN LAMPIRAN SELESAI
    Energa Smart Farming System — Operasional Lapangan Berkelanjutan
```
