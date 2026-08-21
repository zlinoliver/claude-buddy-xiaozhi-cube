# 烧录教程（小白版）— Claude Desktop Buddy · Shuzhi 1.54" TFT 4G

把定制固件刷到你的"小智 CUBE 1.54 TFT 4G"（ESP32-S3）设备上，几分钟搞定。
三种方法，**推荐方法 A（网页烧录，免安装）**。

> **固件从 [GitHub Releases 页面](../../../releases/latest) 下载**：
> - 首次安装用 **`shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin`**（烧到地址 `0x0`）。

---

## 准备工作（所有方法通用）

1. 一根**能传数据的 USB 线**（不是只能充电的），把设备连到电脑。
2. 本设备是 ESP32-S3 原生 USB，**不用按住任何按键**，插上即可烧录。
3. 记住：刷机不会损坏设备，随时能刷回原厂固件（见文末"恢复原厂固件"）。

---

## 方法 A：网页烧录（推荐，最简单，免安装）✅

用 **Chrome 或 Edge 浏览器**（Safari/Firefox 不支持），全程点鼠标。

1. 打开在线烧录器：**https://espressif.github.io/esptool-js/**
2. 波特率保持默认（115200 或 460800），点 **Connect**。
3. 在弹出的串口列表里选你的设备（名字类似 `USB JTAG/serial debug unit` 或 `usbmodemXXXX`），点连接。
4. 在 **Flash Address** 填 `0x0`，点 **Choose File** 选 `shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin`。
5. 点 **Program**，等进度条走完（约 30 秒）。
6. 完成后拔插一下 USB 或按设备电源键重启，设备就跑新固件了。

> 如果发布方（比如项目作者）提供了 **ESP Web Tools 一键安装按钮**（用到 `firmware/manifest.json`），
> 那就更简单：打开网页点一下"Install"即可。

---

## 方法 B：命令行 esptool（适合会用终端的人）

1. 装 Python，然后装 esptool：
   ```bash
   pip install esptool
   ```
2. 找到设备的串口号：
   - macOS： `ls /dev/cu.usbmodem*`
   - Windows：设备管理器里看 `COM` 口（如 `COM5`）
   - Linux： `ls /dev/ttyACM* /dev/ttyUSB*`
3. 烧录（把 `<PORT>` 换成你的串口）：
   ```bash
   esptool.py --chip esp32s3 -p <PORT> -b 460800 \
     write_flash 0x0 firmware/shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin
   ```

> macOS 若报 TLS 证书错误，先执行：
> `export SSL_CERT_FILE=$(python3 -c "import certifi; print(certifi.where())")`

---

## 方法 C：M5Burner（如果你已经在用）

M5Burner 底层也是 esptool，可以烧任意 ESP32 固件到本设备：

1. 打开 M5Burner，切到 **自定义/本地固件（Custom / Burn from local file）**。
2. 选择 `shuzhi-1.54tft-4g-buddy-v0.3.0-merged-8MB.bin`，**烧录地址填 `0x0`**。
3. 选对串口，芯片选 **ESP32-S3**，点 Burn。

> ⚠️ 注意：M5Burner **在线固件商店（搜索列表）是 M5Stack 官方维护的、面向 M5Stack 硬件**，
> 本设备不是 M5Stack 硬件，所以**别去商店里搜这个固件**——用上面的"本地固件"方式烧本仓库的 .bin 即可。
> 关于能不能把本固件上传到 M5Burner 商店，见 `README.md` 的说明。

---

## 首次配对（烧录后）

1. 电脑上打开 **Claude Desktop → Hardware Buddy & Maker Devices** 面板。
2. 点 **扫描/Change**，选中设备（名字类似 `Claude-XXXX`）。
3. 点 **Connect**，把**设备屏幕上显示的 6 位配对码**输到电脑上完成配对。

### 连不上 / 配对卡住？「配对恢复三连」
1. 设备上：长按右侧第 3 个键进 **Settings → Reset pairing**。
2. Claude Desktop 里：点 **Forget**。
3. **Mac 系统设置 → 蓝牙 → 忽略此设备**（← 最容易漏、也最关键）。
4. 再点 Connect，按设备屏幕的配对码重连。

---

## 恢复原厂（小智语音助手）固件

⚠️ 原厂固件属于厂商，**本项目不附带**。所以**在刷本固件之前，先备份你自己设备的原厂固件**，就能随时刷回去：
```bash
# 刷之前先做一次备份（这个文件自己保存好）：
esptool.py --chip esp32s3 -p <PORT> read_flash 0x0 0x800000 my-stock-backup.bin
# 以后想还原：
esptool.py --chip esp32s3 -p <PORT> write_flash 0x0 my-stock-backup.bin
```

---

## 常见问题
- **网页点 Connect 没有设备**：换数据线、换 USB 口；确认用的是 Chrome/Edge。
- **找不到串口**：装一下 CH34x/USB 驱动一般不需要（S3 是原生 USB），先换线换口。
- **烧完黑屏**：短按电源键开机；确认烧的是 `-merged-8MB.bin` 且地址是 `0x0`。
- **想省电**：长时间不用直接**长按底部电源键关机**最省电。
