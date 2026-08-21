<h1 align="center">Claude Desktop Buddy · ESP32</h1>

<p align="center">
  <img src="docs/hero_v1.png" alt="Claude Desktop Buddy for ESP32" width="820">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/ESP--IDF-v5.5.2+-blue">
  <img src="https://img.shields.io/badge/license-MIT-green">
  <img src="https://img.shields.io/badge/BLE-Nordic%20UART-purple">
  <img src="https://img.shields.io/badge/board-shuzhi--1.54tft--4g-orange">
</p>

<p align="center"><b>English</b> · <a href="README.zh-CN.md">中文</a></p>

A hardware companion for **Claude Desktop** over Bluetooth LE. It shows a little
desk pet and live session status, and — most usefully — lets you **approve or deny
Claude's tool requests with a physical button**. Built on ESP-IDF, compatible with
the [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) hardware ecosystem.

> This project has first-class support for the commercial **小智 / XiaoZhi
> "CUBE 1.54″ TFT (4G)"** puck (board `shuzhi-1.54tft-4g`), including **prebuilt
> firmware you can flash in a browser — no build tools required.**

---

## ✨ Features

- **Physical approve / deny** for Claude's tool-permission prompts — top button = Yes, 2nd = No.
- **Live session status** — running / waiting / completed Cowork & Code sessions, token usage.
- **Battery %** shown on screen (calibrated voltage → Li-ion curve).
- **Always-on dim display** — the pet + battery stay glanceable; brightens on activity.
- **Silent + power-saving mode** — dims when idle and duty-cycles the CPU/radio while
  keeping the BLE link alive, so requests still arrive. Power off fully with a long press.
- **On-device pairing reset** + robust BLE discovery.
- **18 ASCII desk pets** and optional **GIF characters** pushed from Claude Desktop.
- **Secure BLE pairing** — LE Secure Connections with a 6-digit passkey.

## 🔌 Supported hardware

This project is **customized and tested for one device only** — the commercial
**小智 / XiaoZhi "CUBE" 1.54″ TFT (4G)** puck:

| Board | Chip | Display | Prebuilt firmware |
|-------|------|---------|:---:|
| **shuzhi-1.54tft-4g** (XiaoZhi CUBE 4G) | ESP32-S3 | ST7789 240×240 | ✅ [Releases](../../releases/latest) |

> Built on the [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) board framework,
> trimmed down to this one device.

## 🚀 Quick start

### A) Flash the prebuilt firmware — no build tools (recommended for the Shuzhi board)
1. Download `shuzhi-1.54tft-4g-buddy-<version>-merged-8MB.bin` from the
   **[Releases page](../../releases/latest)**.
2. Open the browser flasher **<https://espressif.github.io/esptool-js/>** in Chrome/Edge,
   connect the device over USB, and flash the file at address **`0x0`**.
3. Full beginner walkthrough (three methods, in Chinese): **[release/FLASH-GUIDE.md](release/FLASH-GUIDE.md)**.

### B) Build from source
Needs [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/) **v5.5.2+**.
Components download automatically on first build.
```bash
. ~/esp/esp-idf/export.sh
idf.py set-target esp32s3
idf.py build flash monitor
```

## 🔗 Pair with Claude Desktop
1. Open the **Hardware Buddy & Maker Devices** panel in Claude Desktop
   (enable Developer Mode first if needed).
2. Scan, pick your device (advertises as `Claude-XXXX`), click **Connect**.
3. Enter the **6-digit passkey shown on the device screen**.
4. Start a Cowork/Code session — the device shows live status and forwards approval prompts.

### If pairing gets stuck — the reset trilogy
BLE pairing state lives in **three** places; clear all three, then reconnect:
1. **Device** → long-press the Nav button → Settings → **Reset pairing**.
2. **Claude Desktop** → **Forget**.
3. **macOS System Settings → Bluetooth → Forget This Device** (the commonly-missed one —
   the OS keeps its own pairing key that the other two do not clear).

## 🎛️ Controls (Shuzhi 1.54″ TFT 4G)
Three buttons on the right, top → bottom:

| Button | Idle | Approval pending |
|--------|------|------------------|
| **1st (top)** | Cycle pet species | **Approve** |
| **2nd** | Toggle demo mode | **Deny** |
| **3rd (Nav)** | Switch page / **long-press = Settings** | Wake screen to view |
| **4th (bottom)** | Hardware power — **short = on, long = off** | — |

## 🔋 Power & battery (Shuzhi)
- Battery % is read from ADC2/ch6 (GPIO 17), calibrated to millivolts and mapped
  through a Li-ion curve.
- Idle screen dims to a low floor instead of switching off (`BUDDY_SCREEN_IDLE_PCT`
  in `main/buddy/core/buddy_app.cc`; set to `0` for eco screen-off).
- Power management (DFS + BLE modem sleep) reduces idle draw while keeping the link up;
  for the longest standby, **long-press the power button to switch off**.

## 📡 BLE protocol
Implements the Claude Desktop Hardware Buddy protocol: Nordic UART Service (NUS),
newline-delimited JSON, heartbeat snapshots, permission prompt forwarding, GIF push
transfer, and LE Secure Connections with MITM protection.

## 🗂️ Architecture
```
main/
├── boards/                       # Hardware abstraction (per-board pins/display/buttons)
│   └── shuzhi-1.54tft-4g/        # This device (ST7789, battery, buttons, power mgmt)
├── display/                      # LVGL display layer
├── buddy/
│   ├── ble/                      # Nordic UART Service (Bluedroid)
│   ├── core/                     # BuddyApp, TamaState, protocol, power/screen policy
│   ├── ui/                       # LVGL UI (pet, status, settings, battery label)
│   ├── pet/                      # ASCII species + GIF character
│   ├── storage/                  # NVS persistence
│   └── xfer/                     # BLE file transfer (LittleFS)
└── main.cc                       # app entry + power management config
```

## 📚 Docs
- **[release/README.md](release/README.md)** — the flashing / release package.
- **[SHUZHI-1.54TFT-4G-HARDWARE.md](SHUZHI-1.54TFT-4G-HARDWARE.md)** — full reverse-engineered
  hardware reference (pin map, battery ADC, BLE fixes, power-saving internals).

## ⚖️ License & disclaimer
MIT — see [LICENSE](LICENSE). This is an independent, community project. It is **not**
affiliated with or endorsed by Anthropic, the device vendor, or M5Stack. The vendor's
original firmware is **not** redistributed here — back up your own device before flashing.
Flash at your own risk.
