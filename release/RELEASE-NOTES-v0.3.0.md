# Claude Desktop Buddy v0.3.0 — XiaoZhi CUBE 1.54" TFT 4G

First public release. Turns the commercial **小智 / XiaoZhi "CUBE" 1.54″ TFT (4G)**
puck (ESP32-S3, board `shuzhi-1.54tft-4g`) into a **Claude Desktop companion**:
approve/deny Claude's tool requests with a physical button, over Bluetooth LE.

## Flash
Easiest: the browser flasher at <https://espressif.github.io/esptool-js/> — connect
over USB and flash the merged image at address `0x0`. Beginner guide:
[release/FLASH-GUIDE.md](FLASH-GUIDE.md).

| File | Offset | Use |
|------|--------|-----|
| `shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin` | `0x0` | First install / web flasher / M5Burner |
| `shuzhi-1.54tft-4g-buddy-v0.3.0-app.bin` | `0x20000` | App-only update |

## Highlights
- 🔋 On-screen **battery %** (calibrated voltage → Li-ion curve)
- 🌙 **Always-on dim display** + **silent / power-saving** mode (CPU/radio duty-cycle, stays BLE-connected)
- ✅ **Multi-prompt approvals** — handles back-to-back approve/deny requests
- 🔗 On-device **Reset pairing** + fixed BLE discoverability (name in scan response)
- 🔌 Long-press the power button for **true power-off**

## SHA-256
```
f3979339755273fdddeca0819810c3245e8ba9124f661100947d7a632936ecdb  shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin
f5a03f9ace5b87d45d05ff71e4554b75177e4303008db012785621e80d612830  shuzhi-1.54tft-4g-buddy-v0.3.0-app.bin
```

> ⚠️ Independent community project — **not** affiliated with Anthropic, the device
> vendor, or M5Stack. Back up your device's stock firmware before flashing.
