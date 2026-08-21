# Claude Desktop Buddy — Shuzhi 1.54" TFT 4G

A custom firmware that turns the commercial **小智 / XiaoZhi "CUBE 1.54 TFT 4G"**
puck (ESP32-S3) into a **Claude Desktop companion**: it shows a little pet, live
status, and — most usefully — lets you **approve or deny Claude's tool requests
with a physical button**, over Bluetooth LE.

This folder is the **open-source release package** for that device: prebuilt
firmware + a beginner flashing guide. Source and full hardware notes are in the
repository.

![device](../docs/hero_v1.png)

---

## What it does
- **Physical approve / deny** for Claude Desktop tool prompts (top button = Yes, 2nd = No).
- **Battery %** shown top-right (calibrated).
- **Always-on dim display** — the pet + battery stay glanceable; brightens on activity.
- **Silent + power-saving mode** — dims when idle, keeps the BLE link alive so
  requests still arrive; long-press the bottom button to fully power off.
- **On-device pairing reset** and robust BLE discovery.

## Hardware
- Commercial **小智 CUBE 1.54" TFT (4G)** puck — ESP32-S3, 8 MB flash, 8 MB octal PSRAM,
  240×240 ST7789 LCD, 3 side buttons + power button, BLE.
- The 4G/eSIM modem and mic exist on the board but are **not used** by this firmware
  (it talks to the computer over Bluetooth LE only).
- Full reverse-engineered pin map & internals: **[SHUZHI-1.54TFT-4G-HARDWARE.md](../SHUZHI-1.54TFT-4G-HARDWARE.md)**.

## Quick start (flash it)
1. Download **`shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin`** from the
   **[Releases page](../../../releases/latest)**.
2. Follow **[FLASH-GUIDE.md](FLASH-GUIDE.md)** — the easiest path is the browser
   flasher at <https://espressif.github.io/esptool-js/> (flash at address `0x0`).
3. Pair from Claude Desktop → *Hardware Buddy & Maker Devices*, enter the passkey
   shown on the device.

## Build from source
See the repo root. In short (needs ESP-IDF v5.5.x):
```bash
. ~/esp/esp-idf/export.sh
idf.py -DBOARD=shuzhi-1.54tft-4g set-target esp32s3
idf.py -DBOARD=shuzhi-1.54tft-4g build flash monitor
```
Key custom source:
- `main/boards/shuzhi-1.54tft-4g/` — board definition (pins, display, battery, buttons)
- `main/buddy/` — the buddy app (UI, BLE NUS protocol, pet, settings)

## Restore the original firmware
The vendor's stock firmware is **not redistributed here**. Before flashing this
firmware, **back up your own device** so you can always go back:
```bash
esptool.py --chip esp32s3 -p <PORT> read_flash 0x0 0x800000 my-stock-backup.bin   # do this first
esptool.py --chip esp32s3 -p <PORT> write_flash 0x0 my-stock-backup.bin            # restore later
```

## Can I put this on M5Burner?
Short answer: **you can flash it *with* M5Burner (local/custom burn at `0x0`), but
it does not belong in the M5Burner online store.** The store's catalog is
curated by M5Stack for **M5Stack hardware** — this puck is a different vendor's
device, so a store listing isn't the right channel (and users would still need the
correct flash offset). For a one-click beginner experience, prefer an **ESP Web
Tools** page (a `manifest.json` is included in `firmware/`) hosted on GitHub Pages,
or just publish the `.bin` on **GitHub Releases**. See FLASH-GUIDE.md method C.

## License
See the repository `LICENSE`. This is an independent, community customization; it
is **not** affiliated with or endorsed by Anthropic, the device vendor, or M5Stack.
Flash at your own risk.
