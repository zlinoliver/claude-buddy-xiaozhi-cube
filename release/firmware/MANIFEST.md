# Firmware — Claude Desktop Buddy · Shuzhi 1.54" TFT 4G (ESP32-S3)

App version **v0.3.0** · ESP-IDF v5.5.2 · board `shuzhi-1.54tft-4g`

> **The `.bin` files are published on the [GitHub Releases page](../../../../releases/latest), not in the repo.**
> Download them there. This folder keeps only the docs and the web-installer manifest.

| File (on the Releases page) | Flash at | Use |
|------|----------|-----|
| `shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin` | `0x0` | **Easiest** — single full image (bootloader + partition table + otadata + app). Use this for a first install / web flasher / M5Burner. |
| `shuzhi-1.54tft-4g-buddy-v0.3.0-app.bin` | `0x20000` | App only — for devices that already run this project (updates the app partition). |
| `manifest.json` (in this folder) | — | [ESP Web Tools](https://esphome.github.io/esp-web-tools/) manifest for a one-click browser flasher. |

## SHA-256
Each Release build embeds a compile timestamp, so hashes change per build — always
verify against the checksums published on that Release. For **v0.3.0**:
```
f3979339755273fdddeca0819810c3245e8ba9124f661100947d7a632936ecdb  shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin
f5a03f9ace5b87d45d05ff71e4554b75177e4303008db012785621e80d612830  shuzhi-1.54tft-4g-buddy-v0.3.0-app.bin
```

## Maintainer: publishing a Release
```bash
gh release create v0.3.0 release/firmware/*.bin \
  --title "v0.3.0" --notes "See release/firmware/MANIFEST.md"
```

## Flash offsets (for tools that need them, e.g. M5Burner custom burn)
```
0x0      bootloader        (inside the merged image)
0x8000   partition table   (inside the merged image)
0xd000   otadata           (inside the merged image)
0x20000  app               (= the -app.bin)
```
Flash mode `dio`, flash freq `80m`, flash size `8MB`, chip `esp32s3`.

## What's in v0.3.0
- Battery percentage indicator (top-right), calibrated voltage + Li-ion curve.
- Always-on dim display (idle screen dims instead of turning off).
- Silent mode + power saving (DFS + BLE modem sleep; BLE stays connected).
- Multi-prompt approval fix (handles back-to-back approve/deny requests).
- On-device **Reset pairing** in Settings.
- BLE advertising fix (device is discoverable by name after a Forget).

See `../../SHUZHI-1.54TFT-4G-HARDWARE.md` §9–§10 for details.

## Restore the original stock 4G firmware
The vendor's stock image is **not redistributed** — back up your own device before
flashing, then you can always restore it:
```bash
esptool.py --chip esp32s3 -p <PORT> read_flash 0x0 0x800000 my-stock-backup.bin   # do this first
esptool.py --chip esp32s3 -p <PORT> write_flash 0x0 my-stock-backup.bin            # restore later
```
