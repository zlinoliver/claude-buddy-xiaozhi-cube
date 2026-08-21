# Shuzhi 1.54" TFT 4G (xiaozhi-compatible, ESP32-S3)

Commercial xiaozhi "CUBE 1.54 TFT 4G" device. The board definition was **not**
public — this pin map was reverse-engineered from the stock firmware
(`SHUZHI_1_54TFT_4G`, xiaozhi v1.6.6 / ESP-IDF v5.4.1) by disassembling the
flash image.

- **Chip**: ESP32-S3 (rev v0.2)
- **Flash**: 8MB
- **PSRAM**: 8MB Octal
- **Display**: ST7789 240×240 square, SPI3_HOST, spi_mode 3, 80MHz, color inverted
- **4G**: ML307 modem + audio codec (present on hardware, unused by the buddy)

## Pin map

| Signal | GPIO |
|--------|------|
| Display MOSI | 10 |
| Display SCLK | 9 |
| Display CS | 14 |
| Display DC | 8 |
| Display RST | 18 |
| Backlight (PWM) | 13 |
| Power enable | 21 (high) |
| Button 1 / top — **Approve / Yes** | 39 |
| Button 2 — **Deny / No** | 40 |
| Button 3 — Nav (pages; hold = settings) | 0 |
| Button 4 / bottom — power/reset | hardware |

Confirmed physical layout, top→bottom on the right side of the case. The top two
buttons are Yes (top) and No (just below). The on-screen approval hint reads
`Top = Yes   2nd = No`.

## Build & Flash

```bash
idf.py -DBOARD=shuzhi-1.54tft-4g set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

The vendor's stock firmware is proprietary and not redistributed here. **Back up
your own device before flashing**, then you can always restore it:

```bash
esptool.py --chip esp32s3 -p PORT read_flash 0x0 0x800000 my-stock-backup.bin   # do this first
esptool.py --chip esp32s3 -p PORT write_flash 0x0 my-stock-backup.bin           # restore later
```
