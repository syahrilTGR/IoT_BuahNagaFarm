# TROUBLESHOOTING

**Sistem Kontrol Pencahayaan Nirkabel Buah Naga**

---

## Flutter App Issues

### 1. Gradle Lock Timeout
```
Timeout waiting to lock build logic queue...
Lock file: android/.gradle/noVersion/buildLogic.lock
```
**Fix:**
```bash
pkill -9 -f "gradle"
pkill -9 -f "gradlew"
rm -rf android/.gradle
flutter clean && flutter pub get
flutter run
```

### 2. iOS Build: Deployment Target Error
```
The package product 'firebase-auth' requires minimum platform version 15.0,
but this target supports 13.0
```
**Fix:**
```bash
# Update iOS deployment target in Xcode project
cd ios
# Edit Runner.xcodeproj/project.pbxproj
# Change IPHONEOS_DEPLOYMENT_TARGET = 13.0 → 15.0 (3 occurrences)

# OR use flutterfire CLI:
flutterfire configure --platforms=ios
```

### 3. iOS Build: Xcode Workspace Not Found
```
An error occurred when adding Swift Package Manager integration:
Xcode workspace not found.
```
**Fix:**
```bash
cd ios
rm -rf Podfile Podfile.lock Pods Runner.xcworkspace .symlinks
cd ..
flutter clean && flutter pub get
# Xcode will regenerate workspace on next build
```

### 4. CocoaPods vs Swift Package Manager Conflicts
```
All plugins found for ios are Swift Packages, but your project still has CocoaPods integration.
```
**Fix:**
```bash
cd ios
pod deintegrate
rm -rf Podfile Podfile.lock Pods Runner.xcworkspace .symlinks
# Update Debug.xcconfig & Release.xcconfig to only include Generated.xcconfig
cd ..
flutter clean && flutter pub get
flutter build ios --no-codesign
```

### 5. `flutter clean` Takes Long Time
**Normal behavior** — cleans: `build/`, `.dart_tool/`, `android/.gradle/`, iOS ephemeral files. Only run when:
- Changing dependencies in `pubspec.yaml`
- Native code changes
- Unexplained build errors

**Faster alternative:**
```bash
rm -rf build/ .dart_tool/ android/.gradle/
# Skip full clean
```

### 6. Firebase Auth / Config Missing (iOS/macOS)
```
DefaultFirebaseOptions have not been configured for ios
```
**Fix:**
```bash
flutterfire configure --platforms=ios,android,web
# Or if CLI fails, manually copy from firebase_options.dart to iOS config
```

---

## ESP32 / PlatformIO Issues

### 1. Library Not Found (Firebase Arduino Client)
```
Resolving esp32dev dependencies... Library not found
```
**Fix:**
```ini
# platformio.ini - use correct library ID
lib_deps =
  mobizt/Firebase Arduino Client Library for ESP8266 and ESP32@^4.4.17
```

### 2. Timer Struct Initialization Error
```
cannot initialize Timer with aggregate initializer
```
**Fix:** Add explicit constructor to Timer struct:
```cpp
struct Timer {
  unsigned long prev;
  unsigned long interval;
  Timer(unsigned long p, unsigned long i) : prev(p), interval(i) {}
};
```

### 3. Button Not Working (GPIO0)
```
Pin P4 (GPIO0) touched to GND, no response
```
**Cause:** GPIO0 is boot strapping pin (can't use as input).
**Fix:** Use GPIO4 (D4, physical pin 27 on 38-pin DevKit):
```cpp
const int BUTTON_PIN = 4;  // GPIO4, not GPIO0
pinMode(BUTTON_PIN, INPUT_PULLUP);
```

### 4. PZEM-004T No Data
**Check:**
- Wiring: PZEM TX → ESP32 RX2 (GPIO16), PZEM RX → ESP32 TX2 (GPIO17)
- Voltage: PZEM v3.0 = 3.3V logic (direct connect OK, no LLC needed)
- Baud rate: 9600 default

### 5. Firebase Connection Failed (ESP32)
**Check `config.h`:**
- API Key valid?
- Database URL correct format: `https://project.firebaseio.com` (no trailing slash)
- Service Account email/private key correct?
- Firebase project has RTDB enabled?
- Service Account has `roles/firebasedatabase.admin`?

---

## Firebase Issues

### 1. Permission Denied (RTDB/Firestore)
**Check Rules:**
```json
// RTDB - require auth
".read": "auth != null",
".write": "auth != null"
```
**Fix:** Ensure user/device authenticated before read/write.

### 2. Data Not Syncing
**Debug:**
```bash
# Check RTDB in Firebase Console → Realtime Database
# Verify paths: /lamp/control/a, /lamp/status/a
# Use `adb logcat` or Serial Monitor for ESP32 logs
```

### 3. `flutterfire configure` Not Found
```bash
# Add to PATH
export PATH="$PATH:$HOME/.pub-cache/bin"
# Then
flutter pub global activate flutterfire_cli
flutterfire configure
```

---

## Git / Workflow Issues

### 1. `config.h` Accidentally Committed
```bash
# Remove from history (if already pushed)
git filter-branch --force --index-filter \
  'git rm --cached --ignore-unmatch firmware/src/config.h' \
  --prune-empty --tag-name-filter cat -- --all

# Force push (coordinate with team!)
git push origin --force --all
```

### 2. Merge Conflicts in `platformio.ini`
**Prevention:** Use separate branches per feature, small PRs.

### 3. Large Files in Repo (.pio, build/, Pods)
**Fix:** Add to `.gitignore`, then:
```bash
git rm -r --cached .pio build/ ios/Pods/
git commit -m "chore: remove build artifacts from index"
```

---

## Hardware / Wiring Issues

### 1. SSR Not Triggering
- Check LLC output voltage (measure with multimeter)
- SSR input requires 5V/24VDC — LLC must convert 3.3V → 5V
- Verify SSR datasheet: input voltage range

### 2. PZEM Returns NaN / 0
- Swap TX/RX wires
- Check baud rate (9600 default)
- Ensure common ground between PZEM and ESP32

### 3. ESP32 Crashes / Reboots
- Check power supply (5V 2A minimum)
- Brownout detector triggered? Add capacitor on 3.3V line
- Watchdog timeout? Add `yield()` in long loops

---

## Quick Reference Commands

```bash
# Flutter
flutter clean && flutter pub get && flutter run -d <device_id>
flutter build apk --release
flutter build ios --release

# PlatformIO
pio run -e esp32dev          # Build production
pio run -t upload -e esp32dev # Upload production
pio run -e test_platform      # Build test
pio monitor -e test_platform  # Serial monitor

# Firebase
firebase deploy --only database,firestore:rules
firebase emulators:start

# Git
git add . && git commit -m "feat: <brief>" && git push
```

---

*Update this file when new issues discovered.*