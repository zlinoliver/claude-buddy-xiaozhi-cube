# Shuzhi 1.54" TFT 4G — Hardware Reference

Reverse-engineered hardware notes for the commercial xiaozhi device that this
repo's `shuzhi-1.54tft-4g` board targets. Captured 2026-06-25; battery sense +
power-management work added 2026-06-29.

> **Note on the stock firmware variant:** the source path embedded in the stock
> image is `./main/boards/shuzhi-1.54tft-**wifi**/power_manager.h` — i.e. the
> hardware this repo calls `shuzhi-1.54tft-4g` actually shipped the **Wi-Fi**
> variant firmware. The pin map below is unaffected (same PCB), but it explains
> why the stock battery ADC sits on ADC2 (see §3 Power).

> ⚠️ This board was **not** published by the vendor. Everything below (especially
> the GPIO map) was recovered by disassembling the stock firmware flash image —
> see [Reverse-engineering method](#reverse-engineering-method). Display, backlight,
> PSRAM and BLE are **confirmed working on hardware**. The Nav/Approve/Deny →
> physical-key mapping is a best guess and may need swapping in `config.h`.

---

## 1. Device identification

| Item | Value |
|------|-------|
| Marketing name | 无名星智 CUBE 1.54" TFT (4G version) — a xiaozhi AI voice-chat puck |
| Firmware board id | `shuzhi-1.54tft-4g` (C++ class `SHUZHI_1_54TFT_4G`) |
| Stock firmware | **xiaozhi v1.6.6**, ESP-IDF v5.4.1, compiled May 31 2025 |
| BLE / device name (stock) | `XiaoZhi-1.54(4G)` / `Xiaozhi-1325` |
| Form factor | Square 1.54" non-touch LCD, 4 side buttons, 3D-printed case |
| Connectivity | Wi-Fi + **4G (ML307 cellular modem)** — "DualNetworkBoard" |

## 2. SoC / module

| Item | Value |
|------|-------|
| Chip | **ESP32-S3** (QFN56), revision **v0.2** |
| Module class | ESP32-S3R8 (in-package octal PSRAM) |
| CPU | Dual-core Xtensa LX7 @ 240 MHz + LP core |
| Crystal | 40 MHz |
| Radios | Wi-Fi b/g/n, Bluetooth 5 (LE) |
| Flash | **8 MB**, vendor **Boya**, DIO mode @ 80 MHz |
| PSRAM | **8 MB octal** — AP Memory (vendor 0x0D), gen-3, 64 Mbit die, 3V, 80 MHz |
| Base MAC (Wi-Fi/STA) | `fc:01:2c:d8:13:24` |
| BLE MAC | `fc:01:2c:d8:13:26` |

### USB / flashing interface
| Item | Value |
|------|-------|
| Interface | **Native USB-Serial/JTAG** (no external UART bridge) |
| USB VID:PID | `0x303A:0x1001` (Espressif "USB JTAG_serial debug unit") |
| macOS serial port | `/dev/cu.usbmodem11401` |
| Download mode | Automatic — **no BOOT-key hold needed** |
| Console UART (logs) | TX = GPIO 43, RX = GPIO 44 @ 115200 |

---

## 3. GPIO pin map (the important part)

### Display — ST7789, 240×240, SPI
| Signal | GPIO | Notes |
|--------|------|-------|
| SPI host | `SPI3_HOST` | |
| **MOSI / SDA** | **GPIO 10** | |
| **SCLK** | **GPIO 9** | |
| **CS** | **GPIO 14** | |
| **DC / RS** | **GPIO 8** | |
| **RST** | **GPIO 18** | |
| **Backlight** | **GPIO 13** | PWM via LEDC, **not** inverted |
| MISO | — | not connected |

Panel config: `spi_mode = 3`, `pclk = 80 MHz`, `bits_per_pixel = 16`,
`invert_color = true`, `swap_xy = false`, `mirror_x = false`, `mirror_y = false`,
offset `(0, 0)`.

### Buttons — all active-low (physical top→bottom, right side of case — confirmed)
| Position | GPIO | Function |
|----------|------|----------|
| 1st (top) | **GPIO 39** | **Approve / Yes** (changes pet when idle) |
| 2nd | **GPIO 40** | **Deny / No** (toggles demo mode when idle) |
| 3rd | **GPIO 0** | Nav — switch pages; hold = settings (BOOT strapping pin) |
| 4th (bottom) | — | **hardware power latch** (not a GPIO): **short press = power on, long press (~3 s) = power off**. Confirmed on hardware 2026-06-29 — long press cuts power (stays off, no auto-reboot); not a reset. Firmware cannot read or override it. |

On-screen approval hint: `Top = Yes   2nd = No`.

### Power
| Signal | GPIO | Notes |
|--------|------|-------|
| LCD / peripheral power-enable | **GPIO 21** | driven **high** at boot (amp sits on this rail) |
| **Battery voltage sense** | **GPIO 17** | **ADC_UNIT_2, channel 6**, 12-bit, 12 dB atten, raw reads |
| Charging detect | *(unknown)* | a GPIO passed to the stock `PowerManager(gpio_num_t)` ctor; the immediate was set at the board-init call site and not recovered. Not needed to show the percentage. |

**Battery level.** GPIO 17 (ADC2_CH6) does **not** collide with the buddy's GPIO 7
speaker (ADC1_CH6), and since the buddy uses BLE only — no Wi-Fi — ADC2 is fully
reliable here.

> ⚠️ **The stock raw-count formula does not work as a gauge.** The stock
> `ReadBatteryAdcData` maps raw ADC linearly with `level ≈ (avg − 1969) × 100 / 2117`
> (empty anchor ~1969, "full" anchor ~4086). On hardware this **badly
> under-reports**: a full battery measured **pin = 2189 mV → ~4.2 V** (raw ≈ 2573),
> yet the raw formula returned only **28 %** — because the ~4086 "full" anchor sits
> near the ADC full scale and would require ~6 V at the cell, which is unreachable.
> The empty anchor (~1969 ≈ 3.35 V) is fine; only the span/full anchor is wrong.

The buddy therefore **does not** use the raw formula. `GetBatteryLevel()` instead:
1. reads ADC2/ch6 (8-sample average) and converts to millivolts with `adc_cali`
   (curve-fitting calibration);
2. multiplies by the **2:1 resistor divider** (`pin_mv × 2 = Vbat`) — confirmed by
   the full-charge measurement (pin 2189 mV ⇒ ~4.38 V while charging);
3. maps `Vbat` through a Li-ion open-circuit-voltage → SoC curve
   (3.00 V → 0 %, 3.70 V → 30 %, 3.80 V → 50 %, 4.20 V → 100 %).

A freshly-charged cell now reads ~100 %. The mid-range uses a generic Li-ion curve
and may read a few % optimistic; trim `kBatteryDivider` / the curve in the board
file if a discharge shows it off.

### Audio — no I2C codec; speaker is direct I2S, mic is PDM
The stock firmware uses `NoAudioCodecSimplexPdm` (I2S speaker + PDM mic, no
ES8311-style codec). The buddy uses **only the speaker** for approval/celebration
tones via an `I2sBuzzer`; the PDM mic is unused.

| Signal | GPIO | Notes |
|--------|------|-------|
| Speaker **I2S BCLK** | **GPIO 15** | |
| Speaker **I2S WS/LRCLK** | **GPIO 16** | |
| Speaker **I2S DOUT** | **GPIO 7** | to the amplifier |
| mic PDM clk / din | 2 / 3 | unused by the buddy |

Sample rates in the stock firmware: mic 16000, speaker 24000. No separate
amp-enable pin (the amp is powered by the GPIO 21 rail).

### Present on hardware but NOT used by the buddy firmware
The 4G modem (ML307) and the PDM microphone are present but unused; their pins
(other than mic PDM clk=2 / din=3 above) were not reverse-engineered.

### GPIO allocation summary
```
Display:  8 (DC)  9 (SCLK)  10 (MOSI)  13 (BL)  14 (CS)  18 (RST)
Audio:    15 (BCLK)  16 (WS)  7 (DOUT)        [mic PDM 2/3 unused]
Power:    21 (peripheral/amp enable, high)  17 (battery sense, ADC2_CH6)
Buttons:  0 (BOOT/Nav)  39 (Approve)  40 (Deny)
Console:  43/44 (USB-JTAG UART)
Free for the buddy: 1, 4-6, 11-12, 19-20, 35-38, 41-42, 45-48
       (some may be wired to the unused 4G modem — verify before reuse)
```

---

## 4. Flash layout

### Stock xiaozhi partition table (read from the original 8 MB image)
| Name | Type | Offset | Size |
|------|------|--------|------|
| nvs | data/nvs | 0x9000 | 16 KB |
| otadata | data/ota | 0xd000 | 8 KB |
| phy_init | data/phy | 0xf000 | 4 KB |
| model | data/spiffs | 0x10000 | 960 KB (TTS/voice model) |
| factory | app | 0x100000 | 5 MB |

### Buddy partition table (`partitions/v2/8m.csv`)
| Name | Type | Offset | Size |
|------|------|--------|------|
| nvs | data/nvs | 0x9000 | 16 KB |
| otadata | data/ota | 0xd000 | 8 KB |
| phy_init | data/phy | 0xf000 | 4 KB |
| ota_0 | app | 0x20000 | 0x2f0000 (≈3.0 MB) |
| ota_1 | app | — | 0x2f0000 |
| assets | data/spiffs | 0x600000 | 2 MB |

Buddy app size ≈ 2.84 MB → fits ota_0 with ~8 % headroom.

---

## 5. Flash & build

### Prebuilt binaries (no ESP-IDF needed)
Ready-to-flash images live in `firmware/` (see `firmware/README.md`):

| File | Flash at | Use |
|------|----------|-----|
| `shuzhi-1.54tft-4g-buddy-v0.2.3-merged-8MB.bin` | `0x0` | single full-flash image (easiest) |
| `shuzhi-1.54tft-4g-buddy-v0.2.3-app.bin` | `0x20000` | app-only update |

```bash
python -m esptool --chip esp32s3 -p /dev/cu.usbmodem11401 -b 460800 \
  write_flash 0x0 firmware/shuzhi-1.54tft-4g-buddy-v0.2.3-merged-8MB.bin
```

### Build from source (ESP-IDF v5.5.2)

```bash
# one-time per shell: fix macOS python TLS cert + load IDF
export SSL_CERT_FILE=$(/Library/Frameworks/Python.framework/Versions/3.12/bin/python3 -c "import certifi; print(certifi.where())")
. ~/esp/esp-idf/export.sh

# configure for this board, build, flash
idf.py -DBOARD=shuzhi-1.54tft-4g set-target esp32s3
idf.py -DBOARD=shuzhi-1.54tft-4g build
idf.py -p /dev/cu.usbmodem11401 flash monitor
```

> **macOS cert gotcha**: the python.org Python 3.12 ships without a usable CA
> bundle, so ESP-IDF tool downloads *and* component-manager fetches fail with
> `CERTIFICATE_VERIFY_FAILED`. Exporting `SSL_CERT_FILE` to the `certifi` bundle
> fixes both.

### Restore the original 4G voice-assistant firmware
The vendor's stock firmware is **not redistributed here** (it is the vendor's
proprietary image). **Back up your own device first**, before flashing anything:
```bash
# One-time backup of YOUR device (keep this file safe):
esptool.py --chip esp32s3 -p <PORT> read_flash 0x0 0x800000 my-stock-backup.bin
# Restore it later:
esptool.py --chip esp32s3 -p <PORT> write_flash 0x0 my-stock-backup.bin
```

---

## 6. Confirmed-working boot evidence

From the first successful buddy boot:
```
Board: SKU=shuzhi-1.54tft-4g
esp_psram: Found 8MB PSRAM device   (octal, 80 MHz)
spi_flash: detected chip: boya / flash io: dio
LcdDisplay: Turning display on / Adding LCD display     ← no SPI/GPIO errors
i2s_buzzer: I2S buzzer initialized on port 0            ← speaker BCLK15/WS16/DOUT7 OK
Backlight: Set brightness to 75                          ← GPIO 13 PWM, applied from saved setting
buddy_ui: Buddy UI ready
nus: NUS BLE initialized as 'Claude-1326'               ← BLE advertising
```
Display verified upright with correct colors on hardware.

---

## 7. Reverse-engineering method

For anyone needing to re-derive or extend this (e.g. the 4G/codec pins):

1. **Dump flash**: `esptool read_flash 0x0 0x800000 original_full_8MB.bin`.
2. **Decode partitions** at offset `0x8000` with `gen_esp32part.py` → app at `0x100000`.
3. **Parse the app image** with `esptool image_info` → segment load addresses:
   - DROM (strings/rodata) @ `0x3c1b0020` (file offset `0x18`)
   - IROM (code) @ `0x42000020` (file offset `0x210018`)
   - Entry `0x40379b38`
4. **Carve** the DROM and IROM segments and disassemble IROM as raw binary with
   `xtensa-esp32s3-elf-objdump -D -b binary -m xtensa --adjust-vma=0x42000020`.
   objdump resolves `l32r` literal targets, so DROM string pointers are visible
   in the disassembly.
5. **Anchor on strings**: locate `__PRETTY_FUNCTION__` / `ESP_ERROR_CHECK`
   expression strings (`SHUZHI_1_54TFT_4G::InitializeSpi()`,
   `esp_lcd_new_panel_st7789(...)`, `esp_lcd_panel_swap_xy(panel_, DISPLAY_SWAP_XY)`,
   etc.). The code that loads those literals is inside the function of interest.
6. **Read the pins**: GPIO numbers are `movi` immediates stored (`s32i`) into the
   driver config structs just before each `spi_bus_initialize` /
   `esp_lcd_new_panel_io_spi` / `esp_lcd_new_panel_st7789` / `Button::Button` /
   `PwmBacklight::PwmBacklight` call. Account for the `call8` window rotation
   (caller `a10/a11/a12` → callee `a2/a3/a4`) when reading constructor arguments.
7. **Audio**: the same way — find `GetAudioCodec` by the sample-rate literals
   (16000/24000), then read the `NoAudioCodecSimplexPdm(...)` call. Constructor
   args beyond `a7` are passed on the stack (`s32i aN, a1, 0/4`), so the 7th/8th
   args (the PDM mic pins) live there; `a5/a6/a7` held the I2S speaker pins.

### 7a. Battery sense (extension, 2026-06-29)

The battery ADC was recovered later, on a machine **without** the xtensa toolchain,
so a different method was used:

1. `strings` on the stock image surfaced `void PowerManager::ReadBatteryAdcData()`,
   `adc_oneshot_read(adc_handle_, ADC_CHANNEL_6, &adc_value)`, `battery_check_timer`,
   and the source path `./main/boards/shuzhi-1.54tft-wifi/power_manager.h`.
2. Parsed the app image with `esptool image-info` to get segment load addresses
   (DROM `0x3c1b0020`, IROM `0x42000020`).
3. Found the literal-pool words equal to those rodata string addresses inside the
   IROM, which located the `PowerManager` constructor (`0x42024314`) and
   `ReadBatteryAdcData` (`0x42024a80`).
4. Disassembled those two functions with a small **hand-written Xtensa decoder**
   (validated by checking that decoded `l32r` targets matched the known literal
   addresses). Findings:
   - constructor memsets a 12-byte `adc_oneshot_unit_init_cfg_t`, stores
     `unit_id = 1` → **ADC_UNIT_2**; `movi a11,6` → **ADC_CHANNEL_6**;
     `chan_cfg = {atten = 3 (12 dB), bitwidth = 12}`; poll timer
     `esp_timer_start_periodic(…, 1000000)` → 1 s.
   - the level log string `"ADC value:%d average:%ld level:%ld"` plus the division
     magic `0x0c138948` (>>32) resolve to `level = (avg − 1969) × 100 / 2120`.
5. ADC_UNIT_2 + ch6 → **GPIO 17** on ESP32-S3 (ADC1_CH6 would be GPIO 7, which is
   the speaker — confirming the battery cannot be on ADC1 here).
6. **The raw-count level formula turned out to be unusable** (see §3 ⚠️): on a
   bench test a full battery read 28 % through it. Verified by logging calibrated
   pin millivolts (2189 mV → ~4.2 V at the cell via the 2:1 divider). The buddy
   replaced it with a calibrated-voltage + Li-ion-curve gauge instead.

---

## 8. Source files in this repo

- `main/boards/shuzhi-1.54tft-4g/config.h` — the pin `#define`s
- `main/boards/shuzhi-1.54tft-4g/shuzhi_1_54tft_4g.cc` — board implementation
- `main/boards/shuzhi-1.54tft-4g/sdkconfig.board` — 8 MB flash / octal PSRAM / 8m.csv / PM + BLE modem sleep
- `main/main.cc` — `esp_pm_configure()` (DFS + light sleep)
- `main/buddy/core/buddy_app.cc` — battery refresh, always-on dim, silent-mode wake policy, multi-prompt approval fix
- `main/buddy/ui/buddy_ui.cc` — top-right battery label, idle animation freeze, "Reset pairing" settings item
- `main/buddy/ble/nus_service.cc` — BLE NUS service, advertising (name in scan response), bond clearing
- `main/boards/shuzhi-1.54tft-4g/README.md` — short board readme
- Registered in `main/Kconfig.projbuild` and `main/CMakeLists.txt`
- `firmware/` — prebuilt flashable binaries (+ `firmware/README.md`)

## 9. Board customizations applied

Tuned for this square panel (all flashed & confirmed on hardware):

**Board (`shuzhi_1_54tft_4g.cc` / `config.h`)**
- **I2S speaker** — `I2sBuzzer` on BCLK 15 / WS 16 / DOUT 7, exposed via
  `GetBuzzer()`. Gives audible approval (`AttentionTone`) and level-up
  (`CelebrateMelody`) tones.
- **Approval hint** — `GetApprovalHint()` returns `Top = Yes   2nd = No` to match
  the vertical 4-button layout (was the inherited `Right = Yes / Left = No`).
- Boot splash runs at full backlight; the app then applies the saved brightness.

**Shared UI / app (affects all boards, guarded so others are unchanged)**
- **Round-vs-rectangular layout** — added `Board::HasRoundDisplay()` (default
  `false`). Round GC9A01 panels (cuican / moji / moji2 / lilygo-t-circle) override
  it to `true` and keep the 40 px bezel-safe margins; rectangular panels use a
  smaller margin and a full-height, spread-out layout in `buddy_ui.cc` (pet /
  status / clock no longer overlap; info-page hint pinned to the bottom).
- **Multi-line status** — `TamaState::msg` enlarged 24→96 B; the rectangular
  status line is top-anchored and wraps onto multiple lines.
- **Working Sound / Brightness / Volume settings** — the on-device settings menu
  now actually drives hardware:
  - `Sound` gates all buzzer calls in `buddy_app.cc` (`if (buzzer && settings_.sound)`).
  - `Brightness` cycles 20–100 %, applied at startup and persisted (NVS `s_bri`).
  - `Volume` (new menu item) cycles 0–100 % with a preview beep; scales the
    `I2sBuzzer` amplitude, persisted (NVS `s_vol`). `Buzzer::SetVolume()` added to
    the base interface (no-op for buzzers without amplitude control).

**Battery, always-on display, silent mode & power saving (2026-06-29 — flashed & confirmed on hardware)**
- **Battery percentage indicator** — `GetBatteryLevel()` reads ADC2/ch6 (GPIO 17),
  converts to millivolts with `adc_cali`, applies the 2:1 divider, and maps through
  a Li-ion SoC curve (§3 / §7a — the stock raw-count formula under-reported badly).
  `buddy_ui.cc` shows a live `NN%` label top-right (yellow ≤ 30 %, red ≤ 15 %),
  refreshed every 5 s. Added `esp_adc` to `main/CMakeLists.txt`. Verified on
  hardware: a full battery reads 100 % (was 28 % with the raw formula).
- **Always-on dim display** — the idle screen now dims to a 5 % floor
  (`BUDDY_SCREEN_IDLE_PCT` in `buddy_app.cc`) instead of turning off, so the pet +
  battery stay glanceable. Set it to `0` for an eco screen-off mode. Backlight
  showed no flicker under DFS on hardware.
- **Silent mode** — being merely BLE-connected no longer keeps the screen lit; the
  screen only wakes to full brightness on a prompt/approval, a passkey, or a button
  press (3rd/Nav key = "view status"). The BLE link stays up regardless, so
  incoming activity from the desktop still reaches the device.
- **Idle animation freeze** — `buddy_ui.cc` skips pet/particle redraws while idle so
  LVGL has no dirty regions and the CPU can scale down; the last frame is retained
  by the panel controller.
- **Power management** — `CONFIG_PM_ENABLE` + tickless idle + `esp_pm_configure`
  (max 240 / min 80 MHz, `light_sleep_enable = true`) and `CONFIG_BT_CTRL_MODEM_SLEEP`.
  Light sleep self-gates behind the backlight/BLE PM locks, so with the screen lit
  and BLE connected the SoC does DFS only (stable, no flicker, connection kept);
  true CPU light sleep engages only in eco screen-off mode. Config lives in
  `sdkconfig.board` and `main.cc`.

On-device check 2026-06-29 (all OK): battery read **21 %** (yellow), idle dim to
5 % with picture retained, no backlight flicker, 3rd-button wake to full
brightness, and — in silent/dimmed state — an incoming desktop tool request woke
the screen and showed the approve/deny prompt with the BLE link still connected.

**Power-saving usage (two options):**
- *Staying connected* — leave it on. Silent mode + DFS + BLE modem sleep give a
  moderate reduction (CPU 240→80 MHz when idle, radio duty-cycles) while keeping
  the BLE link up so approvals/notifications still arrive. Deep CPU light sleep is
  intentionally **not** reached in this always-on/connected state (the backlight +
  BLE PM locks hold it off to avoid flicker/disconnect); it only engages in eco
  screen-off mode (`BUDDY_SCREEN_IDLE_PCT = 0`).
- *Not using it for a while* — **long-press the bottom power button to switch off**
  (true power cut, near-zero draw — the most effective saving). Trade-off: while
  off it is disconnected, so no notifications until you short-press to power on and
  let it reconnect.

---

## 10. BLE approval & pairing fixes (2026-06-29 — flashed & confirmed)

**Multi-prompt approval fix** (`buddy_app.cc`). Approve/Deny is gated by an
`approval_sent_` flag that was only re-armed when a prompt went present→absent.
Back-to-back desktop requests replace prompt id A directly with id B (never
clearing), so `approval_sent_` stayed `true` and the 2nd+ prompts were stuck on
"Sent!" with dead buttons. Fix: track the prompt **id**; whenever it changes to a
new request, reset `approval_sent_`, restart the response timer, and re-alert.

**BLE advertising / discovery fix** (`buddy/ble/nus_service.cc`). The main ADV
packet had `include_name=true` **and** the 128-bit NUS service UUID — 34 B, over
the 31 B limit — so the name was dropped ("Partial data write into ADV") and the
desktop scan showed "None found" after a Forget. (Bonded reconnect still worked
because it uses the MAC, not name discovery — so it only broke after Forget.) Fix:
main ADV = flags + UUID only; **device name moved to the scan response**.

**Reset pairing menu item** (`buddy_ui.cc`). The on-device Settings menu (long-press
Nav) gained a **"Reset pairing"** entry that calls `nus_clear_bonds()`, so a stuck
pairing can be reset without the computer.

**Pairing recovery — do all three** (a stuck/re-paired link usually needs every one):
1. Device: Settings → **Reset pairing**.
2. Claude Desktop: **Forget**.
3. **macOS System Settings → Bluetooth → Forget This Device** — the commonly-missed
   step. The app's Forget and the device's Reset do **not** clear the pairing key
   macOS holds at the OS level; a stale key makes Connect hang with no passkey and
   no connection event ever reaching the device (it logs nothing). Removing it here
   fixed it. Then Connect and enter the passkey shown on the device.

> Debug note: the ESP32-S3 native USB-Serial/JTAG port renumbers (e.g.
> `usbmodem1401` ↔ `usbmodem1301`) every time the device reboots — auto-detect the
> port rather than hardcoding it.
