# Plan: Notifikasi PLN Mati & Anomali Daya (App-Only)

**Tanggal:** 2026-08-12  
**Status:** Draft  
**Biaya:** 100% free (Spark plan, tanpa Cloud Functions)

---

## Arsitektur

ESP32 (firmware) -> RTDB -> Mobile App (Flutter) -> Local Notification
                                     (saat app foreground/background)

Catatan: Notifikasi HANYA muncul saat app hidup. Jika app ditutup, tidak ada notifikasi.

---

## 1. RTDB Structure

/energy/pln_status: { available: bool, source: "PLN"|"UPS", timestamp: number }
/energy/block_a: { voltage, current, power, energy, timestamp }
/energy/block_b: { voltage, current, power, energy, timestamp }
/config/block_a: { expected_power, lamp_count, tolerance_percent }
/config/block_b: { expected_power, lamp_count, tolerance_percent }

---

## 2. Firmware (ESP32) - SUDAH SELESAI

- /energy/pln_status dikirim tiap 10 detik
- /energy/block_a/power dikirim tiap 10 detik
- /energy/block_b/power dikirim tiap 10 detik

File: src/main.cpp

---

## 3. Mobile App (Flutter) - TODO

### 3.1 Listen PLN Status

FirebaseDatabase.instance.ref('/energy/pln_status').onValue.listen((event) {
  final data = event.snapshot.value as Map?;
  if (data == null) return;
  final available = data['available'] as bool;
  final source = data['source'] as String;
  if (!available) {
    _showNotification('PLN Mati!', 'Beralih ke UPS/Generator');
  } else {
    _showNotification('PLN Kembali', 'Sumber daya kembali ke PLN');
  }
});

### 3.2 Listen Power Anomaly

void checkPowerAnomaly(String blockId, double currentPower) async {
  final configSnap = await FirebaseDatabase.instance.ref('/config/$blockId').once();
  final config = configSnap.snapshot.value as Map?;
  if (config == null) return;
  final expected = config['expected_power'] as double;
  final tolerance = (config['tolerance_percent'] as int) / 100;
  final minPower = expected * (1 - tolerance);
  final maxPower = expected * (1 + tolerance);
  if (currentPower < minPower || currentPower > maxPower) {
    final diff = ((currentPower - expected) / expected * 100).toStringAsFixed(1);
    _showNotification('Anomali Daya $blockId', 'Daya ${currentPower.toStringAsFixed(0)}W (normal: ${expected.toStringAsFixed(0)}W, selisih $diff%)');
  }
}

### 3.3 Config Screen

- Form input: expected_power, lamp_count, tolerance_percent
- Simpan ke /config/block_a dan /config/block_b

### 3.4 Local Notification

FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin;

Future<void> _showNotification(String title, String body) async {
  const AndroidNotificationDetails androidPlatformChannelSpecifics =
    AndroidNotificationDetails('smartnaga', 'SmartNaga Notifications',
      importance: Importance.max, priority: Priority.high);
  const NotificationDetails platformChannelSpecifics =
    NotificationDetails(android: androidPlatformChannelSpecifics);
  await flutterLocalNotificationsPlugin.show(0, title, body, platformChannelSpecifics);
}

---

## 4. Dependencies (pubspec.yaml)

dependencies:
  firebase_database: ^10.0.0
  flutter_local_notifications: ^16.0.0

---

## 5. Implementation Order

1. Listen RTDB (PLN status + power) - ~2 jam
2. Anomaly detection logic - ~1 jam
3. Config screen - ~1 jam
4. Local notification - ~30 menit
5. Testing - ~1 jam

Total: ~5-6 jam kerja

---

## 6. Testing Checklist

- [ ] PLN nyala -> app tidak notif
- [ ] PLN cabut -> app notif "PLN Mati!"
- [ ] PLN nyala lagi -> app notif "PLN Kembali"
- [ ] Power normal -> tidak ada notif
- [ ] Lampu mati (power turun >15%) -> notif anomali
- [ ] Config expected_power diubah -> detection update otomatis
- [ ] App ditutup -> tidak ada notifikasi (expected behavior)
