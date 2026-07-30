# Firebase Security Rules

**Sistem Kontrol Pencahayaan Nirkabel Buah Naga**

---

## Realtime Database (RTDB)

```json
{
  "rules": {
    "lamp": {
      "control": {
        ".read": "auth != null",
        ".write": "auth != null",
        "a": {
          ".validate": "newData.isBool()"
        },
        "b": {
          ".validate": "newData.isBool()"
        }
      },
      "status": {
        ".read": "auth != null",
        ".write": "auth != null",
        "a": {
          ".validate": "newData.hasChildren(['state', 'timestamp'])",
          "state": { ".validate": "newData.isBool()" },
          "timestamp": { ".validate": "newData.isNumber()" }
        },
        "b": {
          ".validate": "newData.hasChildren(['state', 'timestamp'])",
          "state": { ".validate": "newData.isBool()" },
          "timestamp": { ".validate": "newData.isNumber()" }
        }
      }
    },
    "device": {
      ".read": "auth != null",
      ".write": "auth != null",
      "online": { ".validate": "newData.isBool()" },
      "last_seen": { ".validate": "newData.isNumber()" },
      "firmware_version": { ".validate": "newData.isString()" }
    },
    "energy": {
      ".read": "auth != null",
      ".write": "auth != null",
      "a": {
        ".validate": "newData.hasChildren(['voltage', 'current', 'power', 'energy'])"
      },
      "b": {
        ".validate": "newData.hasChildren(['voltage', 'current', 'power', 'energy'])"
      }
    }
  }
}
```

### RTDB Path Summary

| Path                        | Type   | Read  | Write | Validation                   |
|-----------------------------|--------|-------|-------|------------------------------|
| `/lamp/control/{a,b}`      | bool   | auth  | auth  | Boolean only                 |
| `/lamp/status/{a,b}`      | object | auth  | auth  | {state: bool, timestamp: num}|
| `/device/online`           | bool   | auth  | auth  | Boolean only                 |
| `/device/last_seen`        | number | auth  | auth  | Number only                  |
| `/energy/{a,b}`           | object | auth  | auth  | {voltage, current, power, energy} |

---

## Firestore Rules

```
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {

    // Logs collection - historical energy data
    match /logs/{logId} {
      allow read, write: if request.auth != null;

      // Entries subcollection
      match /entries/{entryId} {
        allow read, write: if request.auth != null;
        allow create: if request.auth != null
          && request.resource.data.keys().hasAll([
            'lamp_state_a', 'lamp_state_b',
            'power_a_watts', 'power_b_watts',
            'voltage_a', 'voltage_b',
            'current_a', 'current_b',
            'energy_a_kwh', 'energy_b_kwh',
            'timestamp', 'device_id'
          ]);
      }
    }

    // Device config collection
    match /devices/{deviceId} {
      allow read, write: if request.auth != null;
    }

    // User settings collection
    match /users/{userId} {
      allow read, write: if request.auth != null && request.auth.uid == userId;
    }
  }
}
```

---

## Deploy Commands

```bash
# Deploy RTDB Rules
firebase deploy --only database

# Deploy Firestore Rules
firebase deploy --only firestore:rules

# Deploy both
firebase deploy --only database,firestore:rules

# Test rules locally (emulator)
firebase emulators:start
```

---

## Security Notes

1. **Never allow unauthenticated access** — semua operasi butuh `auth != null`
2. **ESP32 uses Service Account** — register device dengan unique token
3. **Web/Mobile uses Firebase Auth** — email/password atau anonymous auth
4. **Rate limiting** — gunakan Firebase App Check untuk production
5. **Data validation** — rules wajib validate tipe data (bool, number, string)
6. **Audit trail** — Firestore logs otomatis untuk semua perubahan

---

## ESP32 Authentication

ESP32 menggunakan **Service Account JSON** untuk akses Firebase:

```cpp
// Di config.h (GITIGNORED)
#define FIREBASE_API_KEY "your-api-key"
#define FIREBASE_DATABASE_URL "https://your-project.firebaseio.com"
#define FIREBASE_PROJECT_ID "your-project-id"
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-xxx@your-project.iam.gserviceaccount.com"
#define FIREBASE_PRIVATE_KEY "-----BEGIN PRIVATE KEY-----\n..."
```

**Cara register:**
1. Firebase Console → Project Settings → Service Accounts
2. Generate new private key (JSON)
3. Extract values ke `config.h` (jangan commit ke git!)

---

## Testing Rules

### RTDB Test
```bash
# Test write (harus berhasil)
curl -X PUT \
  -d '{"a": true}' \
  "https://your-project.firebaseio.com/lamp/control.json?auth=TOKEN"

# Test write (harus gagal - wrong type)
curl -X PUT \
  -d '{"a": "not-a-bool"}' \
  "https://your-project.firebaseio.com/lamp/control.json?auth=TOKEN"
```

### Firestore Test
```bash
# Test write (harus berhasil)
curl -X POST \
  -H "Content-Type: application/json" \
  -d '{
    "lamp_state_a": true,
    "power_a_watts": 12.5,
    "timestamp": "2026-07-30T12:00:00Z"
  }' \
  "https://firestore.googleapis.com/v1/projects/your-project/databases/(default)/documents/logs"
```

---

*Rules ini untuk development. Untuk production, tambahkan Firebase App Check dan rate limiting.*