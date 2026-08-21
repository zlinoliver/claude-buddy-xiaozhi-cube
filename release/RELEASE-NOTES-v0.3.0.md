# Claude Desktop Buddy v0.3.0 — XiaoZhi CUBE 1.54" TFT 4G

First public release. Turns the commercial **小智 / XiaoZhi "CUBE" 1.54″ TFT (4G)**
puck (ESP32-S3, board `shuzhi-1.54tft-4g`) into a **Claude Desktop companion**:
approve/deny Claude's tool requests with a physical button, over Bluetooth LE.

## Which file? (almost everyone: the first one)
👉 **Just download `shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin` and flash it at `0x0`.**
That's the complete firmware. The `-app.bin` is only for developers updating an
existing install — ignore it if you're unsure.

| File | Flash at | Who / when |
|------|----------|-----------|
| ⭐ `shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin` | `0x0` | **Everyone — first install or reflash.** Complete image (bootloader + partitions + app). |
| `shuzhi-1.54tft-4g-buddy-v0.3.0-app.bin` | `0x20000` | Developers only — update just the app on a device already running this firmware. |

## How to flash (≈3 min, no install)
1. Open **<https://espressif.github.io/esptool-js/>** in **Chrome or Edge**.
2. Plug the device into USB, click **Connect**, pick the serial port.
3. Set Flash Address `0x0`, choose the **merged** `.bin`, click **Program**.
4. When it finishes, unplug/replug USB (or press the power button) to reboot.
5. Pair from Claude Desktop → *Hardware Buddy* → enter the passkey shown on the device.

Full step-by-step (command line, M5Burner, pairing recovery):
[release/FLASH-GUIDE.md](FLASH-GUIDE.md).

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
