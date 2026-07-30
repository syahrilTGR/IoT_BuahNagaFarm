# Claude Configuration

## Project Rules
- Use **PlatformIO** for ESP32 firmware in `firmware/` directory
- Use **Flutter** for mobile app in `/Volumes/Data Shared/Project/magang/smartnaga_app`
- Firebase as **RTDB** (real-time control + monitoring) + **Firestore** (historical logs)
- Stay within fork `claudecode` / `gitbr/change`
- Only modify RTDB schema with team consensus

## Library Preferences
- PIO plugins: `pio lib install"
- flutter pub dependencies: local packages
- Focus: equipment that works

## Command Preferences
- publish updates via
  ```bash
  git add . && git commit -m "feat: <feature brief>" && git push branch
  ```
- Flutter
  ```bash
  flutter clean && flutter pub get && flutter run
  ```

## Limitations
- Linting non-strict, focus on critical bugs
- Respond user needs
- submit via open pull requests
- Work in monospace font
- Use Claude-Fable-5 (intellektual terbaik)

## Workflow
- Branch names: `feature/<task_id>/<brief>`
- Code review: 1 PR owner, 1 peer
- Test requirement: fix bugs before PR
- Documentation requirement: doc comments / README

## Instructions
- Respond promptly 1 hours
- Use terminal directly
- No forecasting
- Prioritize equipment, test responses 1 hours