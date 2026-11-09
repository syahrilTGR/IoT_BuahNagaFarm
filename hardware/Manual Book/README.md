# DOKUMENTASI & PANDUAN MANUAL BOOK ALAT SMARTNAGA
**Direktori:** `hardware/Manual Book/`

Folder ini berisi dokumen manual dan panduan operasional perangkat SmartNaga (Sistem Kontrol & Monitoring Lampu Buah Naga Berbasis IoT). Dokumen dipisahkan berdasarkan target pembaca untuk kenyamanan penggunaan:

---

### 1. Untuk Petani, Pemilik Kebun, & Operator Lapangan (USER POV)
* 📖 **[BUKU_PANDUAN_PENGGUNA.md](BUKU_PANDUAN_PENGGUNA.md)**  
  *Panduan lengkap dari sudut pandang pengguna (User Point of View).* Ditulis dengan bahasa praktis, mudah dipahami, tanpa istilah teknis mikrokontroler yang membingungkan. Membahas:
  - Cara menyalakan alat pertama kali.
  - Cara menyambungkan alat ke WiFi / modem kebun lewat HP (3 menit).
  - Cara membaca layar kecil OLED di box panel.
  - Arti bunyi alarm suara (*beep* pendek vs panjang).
  - Cara memakai aplikasi smartphone (kontrol manual, jadwal otomatis, & input jumlah bohlam).
  - Panduan tindakan saat ada lampu mati di tiang kebun atau listrik PLN padam.
  - Tanya jawab umum (FAQ).

* 📋 **[LEMBAR_PANDUAN_RINGKAS_PANEL.md](LEMBAR_PANDUAN_RINGKAS_PANEL.md)**  
  *Lembar SOP 1 Halaman Siap Cetak & Dilaminating.* Dirancang untuk ditempel pada bagian dalam pintu box panel di kebun sebagai referensi cepat bagi pekerja harian.

---

### 2. Untuk Teknisi, Tim Hardware, & Akademisi (ENGINEERING & TECHNICAL POV)
* 🛠️ **[MANUAL_BOOK_SMARTNAGA.md](MANUAL_BOOK_SMARTNAGA.md)**  
  *Buku manual teknis komprehensif.* Berisi spesifikasi teknik mendalam, Bill of Materials (BOM), diagram pengkabelan (*wiring pinout* ESP32), kalibrasi sensor ZMPT101B & PZEM-004T, arsitektur kelistrikan dual-bus (PLN vs UPS), struktur payload Firebase RTDB, serta prosedur update firmware (PlatformIO USB & ArduinoOTA).

