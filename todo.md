pelajari:
- millis()
- rangkaian pzem ke esp32

note:
- yg ke SSR itu yg line phase
- esp32 ada berapa uart
- nyalanya lampu tu ngga semalaman
- ssr udah bisa, dari esp langsung ssr, sebelumnya pake LLC, gabisa, kyk nya LLC nya rusak, di sisi 5v ngga mau ke state low

di esp
- daya di oled buat atas bawah aja

di App
- tambah estimasi rup
- coba ekspor historis dari firestore ke spreadsheet/csv
- di setting, tambahkan inputan daya dan jumlah lampu default, yang nantinya disimpan ke firebase dan digunakan sebagai acuan kondisi normal atau tidak
- notif lampu padam (dari ZMPT), nanti source nya berubah jadi UPS

di RTDB
- add tugangan zmpt ke firebase
---

## Firmware
- [x] PZEM B baca data
- [x] SSR B pin dipindah ke GPIO27
- [x] Timestamp pakai NTP epoch seconds
- [x] Firmware hanya tulis `/energy/block_{a,b}`, app handle `/lamp/status/{a,b}`
- [x] Debounce Firebase command (2s) cegah flicker saat SSL reconnect
- [x] Boot: semua lamp OFF, baca status dari RTDB setelah connect
- [ ] Lamp B bisa dikontrol dari app (sudah di code, perlu test)

## App (SmartNaga)
- [ ] Align timestamp: firmware pakai epoch seconds → app parse `DateTime.fromMillisecondsSinceEpoch(timestamp * 1000)`
- [ ] `getEnergyStream()` hanya baca `/energy/block_a` → tambah stream `/energy/block_b`
- [ ] `home_bloc.dart`: tambah `energyB` + `lampBOn` ke state (sudah ada lampBOn, perlu energyB)
- [ ] `home_screen.dart`: tambah metrics grid Blok B + lamp toggle Blok B
- [ ] `control_screen.dart`: sudah OK (both blocks)
- [ ] `historical_screen.dart`: tambah filter chip (Harian, Mingguan, Bulanan, Tahunan)
- [ ] `settings_screen.dart`: align dengan design
