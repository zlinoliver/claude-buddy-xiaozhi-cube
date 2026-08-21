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

<p align="center"><a href="README.md">English</a> · <b>中文</b></p>

一个通过蓝牙 LE 与 **Claude Desktop** 配合的硬件伴侣：屏幕上显示桌宠和实时会话状态，
最实用的是——可以**用实体按键给 Claude 的工具请求点"同意/拒绝"**。基于 ESP-IDF，
兼容 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) 硬件生态。

> 本项目对商用的**小智 / XiaoZhi "CUBE 1.54″ TFT (4G)"** 设备（板级 `shuzhi-1.54tft-4g`）
> 做了完整适配，并提供**可在浏览器里一键烧录的预编译固件——无需任何开发工具。**

---

## ✨ 功能

- **实体同意 / 拒绝**：Claude 的工具授权请求——顶键 = 同意，第 2 键 = 拒绝。
- **实时会话状态**：正在运行 / 等待 / 已完成的 Cowork & Code 会话、Token 用量。
- **电量百分比**：屏幕显示（校准电压 → 锂电曲线）。
- **常亮微光屏**：桌宠 + 电量随时可瞄一眼，有活动时自动变亮。
- **静默 + 省电模式**：空闲时变暗、CPU/射频间歇休眠，同时**保持蓝牙连接**，消息照收；
  长时间不用可长按电源键彻底关机。
- **设备端自助重置配对** + 更稳的蓝牙发现。
- **18 种 ASCII 桌宠**，以及可从 Claude Desktop 推送的 **GIF 角色**。
- **安全配对**：LE Secure Connections，6 位配对码。

## 🔌 支持的硬件

本项目**只为一款设备定制并测试过**——商用的**小智 / XiaoZhi "CUBE" 1.54″ TFT (4G)**:

| 板子 | 芯片 | 屏幕 | 预编译固件 |
|-------|------|---------|:---:|
| **shuzhi-1.54tft-4g**（小智 CUBE 4G） | ESP32-S3 | ST7789 240×240 | ✅ [Releases](../../releases/latest) |

> 基于 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) 的板级框架，已精简为只保留这一款设备。

## 🚀 快速上手

### A) 烧预编译固件——免开发工具（Shuzhi 板推荐）
1. 从 **[Releases 页面](../../releases/latest)** 下载
   `shuzhi-1.54tft-4g-buddy-<版本>-merged-8MB.bin`。
2. 用 Chrome/Edge 打开网页烧录器 **<https://espressif.github.io/esptool-js/>**，
   USB 连接设备，把文件烧到地址 **`0x0`**。
3. 小白详细教程（三种方法）：**[release/FLASH-GUIDE.md](release/FLASH-GUIDE.md)**。

### B) 从源码编译
需要 [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/) **v5.5.2+**，
首次编译会自动下载依赖组件。
```bash
. ~/esp/esp-idf/export.sh
idf.py set-target esp32s3
idf.py build flash monitor
```

## 🔗 与 Claude Desktop 配对
1. 在 Claude Desktop 打开 **Hardware Buddy & Maker Devices** 面板（必要时先开 Developer Mode）。
2. 扫描，选中你的设备（广播名类似 `Claude-XXXX`），点 **Connect**。
3. 输入**设备屏幕上显示的 6 位配对码**。
4. 开一个 Cowork/Code 会话——设备就会显示实时状态并转发授权请求。

### 配对卡住？「重置三连」
蓝牙配对状态存在**三个**地方，全清掉再重连：
1. **设备** → 长按 Nav 键 → Settings → **Reset pairing**。
2. **Claude Desktop** → **Forget**。
3. **Mac 系统设置 → 蓝牙 → 忽略此设备**（最容易漏——系统层存了另一把密钥，前两步清不掉它）。

## 🎛️ 按键（Shuzhi 1.54″ TFT 4G）
右侧三个键，从上到下：

| 按键 | 空闲时 | 有待审批时 |
|--------|------|------------------|
| **第 1 键（顶）** | 切换桌宠 | **同意** |
| **第 2 键** | 开关演示模式 | **拒绝** |
| **第 3 键（Nav）** | 切换页面 / **长按 = 设置** | 唤醒屏幕查看 |
| **第 4 键（底）** | 硬件电源——**短按开机，长按关机** | — |

## 🔋 电量与省电（Shuzhi）
- 电量从 ADC2/ch6（GPIO 17）读取，校准成毫伏后按锂电曲线换算百分比。
- 空闲时屏幕降到低亮度地板而不是熄灭（`main/buddy/core/buddy_app.cc` 里的
  `BUDDY_SCREEN_IDLE_PCT`；设为 `0` 则空闲息屏更省电）。
- 电源管理（DFS + BLE modem sleep）在保持连接的前提下降低空闲功耗；
  想最省电、长时间不用时**长按电源键关机**。

## 📡 BLE 协议
实现 Claude Desktop Hardware Buddy 协议：Nordic UART Service (NUS)、换行分隔的 JSON、
心跳快照、授权请求转发、GIF 推送传输，以及带 MITM 保护的 LE Secure Connections。

## 🗂️ 代码结构
```
main/
├── boards/                       # 硬件抽象（各板的引脚/屏幕/按键）
│   └── shuzhi-1.54tft-4g/        # 本设备（ST7789、电量、按键、电源管理）
├── display/                      # LVGL 显示层
├── buddy/
│   ├── ble/                      # Nordic UART Service（Bluedroid）
│   ├── core/                     # BuddyApp、TamaState、协议、电源/屏幕策略
│   ├── ui/                       # LVGL 界面（桌宠、状态、设置、电量标签）
│   ├── pet/                      # ASCII 桌宠 + GIF 角色
│   ├── storage/                  # NVS 持久化
│   └── xfer/                     # BLE 文件传输（LittleFS）
└── main.cc                       # 程序入口 + 电源管理配置
```

## 📚 文档
- **[release/README.md](release/README.md)** —— 烧录 / 发布包说明。
- **[SHUZHI-1.54TFT-4G-HARDWARE.md](SHUZHI-1.54TFT-4G-HARDWARE.md)** —— 完整的硬件逆向参考
  （引脚图、电量 ADC、BLE 修复、省电机制内幕）。

## ⚖️ 许可与免责声明
MIT，见 [LICENSE](LICENSE)。本项目为独立的社区项目，**与 Anthropic、设备厂商、M5Stack
均无关联、亦未获其背书**。厂商原厂固件**不随本项目分发**——刷机前请先备份你自己的设备。
刷机风险自负。
