# console_detect - Linux ARM64 设备检测工具

用于 ArkOS4Clone 项目的设备信息检测工具，可自动识别手持游戏设备并输出其硬件配置信息。

## 功能特点

- 自动检测设备型号
- 获取屏幕分辨率
- 识别摇杆配置（单摇杆/双摇杆/无摇杆）
- 获取热键类型
- 读取屏幕旋转角度
- 识别 LED 类型
- 获取 OTG 类型（auto/manual）
- 检测系统版本（ArkOS4Clone/dArkOS4Clone/NULL）

## 编译

### 本地编译 (x86_64)

```bash
make native
# 或
make
```

### ARM64 交叉编译

```bash
make arm64
# 或
make CROSS=1
```

### 安装

```bash
sudo make install
```

程序将被安装到 `/usr/local/bin/console_detect`。

## 使用方法

### 基本用法

```bash
./console_detect
```

输出示例：
```
========== 设备信息 ==========
设备名称:     r36s
屏幕分辨率:   640 x 480
摇杆数量:     2
热键类型:     happy5
屏幕旋转:     0 度
LED类型:      unsupported
OTG类型:      auto
系统版本:     ArkOS4Clone
==============================
```

### 命令行选项

| 选项 | 长选项 | 说明 |
|------|--------|------|
| `-h` | `--help` | 显示帮助信息 |
| `-j` | `--json` | JSON 格式输出 |
| `-s` | `--shell` | Shell 变量格式输出 |
| `-n` | `--name` | 仅输出设备名称 |
| `-r` | `--resolution` | 仅输出分辨率 (格式: 宽x高) |
| `-k` | `--joystick` | 仅输出摇杆数量 |
| `-t` | `--hotkey` | 仅输出热键类型 |
| `-o` | `--rotation` | 仅输出屏幕旋转角度 |
| `-l` | `--led` | 仅输出 LED 类型 |
| `-O` | `--otg` | 仅输出 OTG 类型 |
| `-b` | `--bootini` | 仅输出 boot.ini 检测的设备名称 |
| `-V` | `--version` | 仅输出系统版本 |

### 输出格式示例

#### JSON 格式 (`-j`)

```bash
./console_detect -j
```

```json
{
  "device_name": "r36s",
  "screen_width": 640,
  "screen_height": 480,
  "joystick_count": 2,
  "hotkey_type": "happy5",
  "rotation": 0,
  "led_type": "unsupported",
  "otg_type": "auto",
  "os_version": "ArkOS4Clone"
}
```

#### Shell 变量格式 (`-s`)

```bash
./console_detect -s
```

```
DEVICE_NAME=r36s
SCREEN_WIDTH=640
SCREEN_HEIGHT=480
JOYSTICK_COUNT=2
HOTKEY_TYPE=happy5
SCREEN_ROTATION=0
LED_TYPE=unsupported
OTG_TYPE=auto
OS_VERSION=ArkOS4Clone
```

可在 Shell 脚本中使用：

```bash
eval $(./console_detect -s)
echo "当前设备: $DEVICE_NAME"
echo "分辨率: ${SCREEN_WIDTH}x${SCREEN_HEIGHT}"
```

#### 单项输出

```bash
# 仅获取设备名称
./console_detect -n
# 输出: r36s

# 仅获取分辨率
./console_detect -r
# 输出: 640x480

# 仅获取摇杆数量
./console_detect -k
# 输出: 2

# 仅获取OTG类型
./console_detect -O
# 输出: auto

# 仅获取系统版本
./console_detect -V
# 输出: ArkOS4Clone
```

## 配置文件

程序从 `/boot/.console` 文件读取设备名称。如果该文件不存在，会自动从 `/boot/boot.ini` 解析 DTB 文件名并推断设备型号，然后将结果写入 `.console` 文件。

### 手动指定设备

可以手动创建 `/boot/.console` 文件：

```bash
echo "r36s" > /boot/.console
```

## 系统版本检测

程序从 `/etc/os-release` 文件读取 `PRETTY_NAME` 字段来检测系统版本：

| PRETTY_NAME 值 | 输出 |
|----------------|------|
| `Ubuntu 19.10` | `ArkOS4Clone` |
| `Debian GNU/Linux 13 (trixie)` | `dArkOS4Clone` |
| 其他值 | `NULL` |
| 文件不存在或解析失败 | `Unknown` |

## 支持的设备

设备配置顺序与 [dtb_selector.go](https://github.com/lcdyk0517/arkos4clone) 的 `Consoles` 列表保持一致，按品牌分组。

### YMC

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| a10mini | 480p | 无 | happy5 | 0 | unsupported | auto |
| a10miniv4 | 540p | 无 | happy5 | 180 | unsupported | auto |

### UDT

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| r36ultra | 720p | 双 | happy5 | 0 | r36ultra | auto |
| r36ultrax | 768p | 双 | happy5 | 0 | ws2812 | auto |

### AISLPC

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| k36s | 480p | 单 | happy5 | 0 | mcu_led | auto |
| r36t | 480p | 单 | happy5 | 0 | mcu_led | auto |
| r36tmax | 720p | 双 | happy5 | 0 | mcu_led | auto |

### GUSGU

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| h7 | 768p | 单 | happy5 | 0 | ws2812 | auto |

### Lenovo

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| go2 | 768p | 单 | happy5 | 0 | ws2812 | auto |

### MagicX

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| xu10 | 480p | 无 | happy5 | 0 | unsupported | auto |

### Batlexp

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| g350 | 480p | 双 | happy5 | 0 | unsupported | auto |

### Kinhank

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| k36 | 480p | 双 | happy5 | 0 | unsupported | auto |

### Anbernic

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| rg351mp | 480p | 双 | select | 0 | unsupported | auto |
| rg351p | 320p | 单 | select | 270 | unsupported | auto |
| rg351v | 480p | 单 | happy5 | 0 | unsupported | auto |

### RetroBox

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| rp1 | 480p | 单 | happy5 | 0 | unsupported | auto |

### Powkiddy

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| rgb10 | 320p | 单 | select | 270 | unsupported | auto |
| rgbv10 | 320p | 无 | select | 270 | unsupported | auto |
| rgb10x | 480p | 单 | happy5 | 0 | unsupported | auto |
| rgb10max1 | 854p480 | 双 | happy5 | 270 | unsupported | auto |
| rgb10max2 | 854p480 | 双 | happy5 | 270 | unsupported | auto |
| rgb20s | 480p | 双 | happy5 | 0 | unsupported | auto |

### Clone R36s / Soysauce R36s

以下 DTB 变体未在 `dtb_mapping` 中显式列出，检测时由 `read_boot_ini` 的 fallback 机制默认映射到 `r36s`：

`rk3326-r36s-type1*` / `rk3326-r36s-type2*` / `rk3326-r36s-type3-panel*` / `rk3326-r36s-type5` / `rk3326-r36s-panel0~4*` / `rk3326-r36s-v21` / `rk3326-r36s-sauce-panel1~5`

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| r36s | 480p | 双 | happy5 | 0 | unsupported | auto |

### GameConsole

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| r46h | 768p | 双 | select | 0 | unsupported | auto |
| r40xxpromax | 768p | 双 | happy5 | 0 | unsupported | auto |
| r40xx | 768p | 双 | happy5 | 0 | unsupported | auto |
| r36hpromax | 768p | 双 | happy5 | 0 | unsupported | auto |
| r45h | 768p | 双 | happy5 | 0 | unsupported | auto |
| r36splus | 720p | 双 | happy5 | 0 | unsupported | auto |
| r33s | 480p | 无 | select | 0 | unsupported | auto |
| r36xx | 480p | 双 | select | 0 | unsupported | auto |
| o30s | 480p | 双 | happy5 | 0 | unsupported | auto |
| r36h | 480p | 双 | select | 0 | unsupported | auto |
| r50s | 854p480 | 双 | happy5 | 270 | unsupported | auto |
| r50h | 720p1280 | 双 | happy5 | 270 | unsupported | auto |

### Diium (SZDiiER)

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| dr28s | 480p | 无 | happy5 | 270 | unsupported | auto |
| d007 | 480p | 双 | select | 0 | dual-gpio | auto |

### XiFan HandHelds

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| mymini | 480p | 单 | select | 0 | gpio | auto |
| mini40 | 720p | 单 | select | 0 | gpio | auto |
| r36max | 720p | 双 | happy5 | 0 | unsupported | auto |
| r36pro | 480p | 双 | happy5 | 0 | unsupported | auto |
| xf35h | 480p | 双 | select | 0 | mcu_led | auto |
| rf35h | 480p | 双 | select | 0 | mcu_led | auto |
| xf40h | 720p | 双 | select | 0 | mcu_led | auto |
| rf40h | 720p | 双 | select | 0 | mcu_led | auto |
| dc35v | 480p | 双 | happy5 | 0 | ws2812 | auto |
| dc40v | 720p | 双 | happy5 | 0 | ws2812 | auto |
| xf40v | 720p | 双 | happy5 | 0 | ws2812 | auto |
| xf28 | 480p | 单 | select | 90 | ws2812 | auto |
| r36max2 | 768p | 双 | happy5 | 0 | ws2812 | manual |
| xf45v | 768p | 双 | happy5 | 0 | ws2812 | manual |
| dc45v | 768p | 双 | happy5 | 0 | ws2812 | manual |
| rf45v | 768p | 双 | happy5 | 0 | ws2812 | manual |
| rf45h | 768p | 双 | happy5 | 0 | ws2812 | manual |
| rf55h | 720p1280 | 双 | happy5 | 90 | ws2812 | manual |

### Other

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | 旋转 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|------|----------|----------|
| hg36 | 480p | 双 | happy5 | 0 | unsupported | auto |
| rx6h | 480p | 双 | select | 0 | unsupported | auto |
| xgb36 | 480p | 单 | happy5 | 0 | gpio | auto |
| t16max | 720p | 双 | happy5 | 0 | unsupported | auto |
| u8 | 800p480 | 双 | happy5 | 270 | unsupported | auto |
| rg36 | 480p | 双 | happy5 | 0 | unsupported | auto |
| rg36pro | 480p | 双 | happy5 | 0 | single-gpio | auto |
| r40s | 800p480 | 双 | happy5 | 270 | unsupported | auto |

## 作为库使用

可以将 `console_detect` 作为库集成到其他 C 程序中：

```c
#include "console_detect.h"
#include <stdio.h>

int main() {
    DeviceInfo info;
    
    if (get_device_info(&info) != 0) {
        fprintf(stderr, "无法获取设备信息\n");
        return 1;
    }
    
    printf("设备: %s, 分辨率: %dx%d, OTG: %s, 系统版本: %s\n", 
           info.device_name, 
           info.screen_width, 
           info.screen_height,
           info.otg_type,
           info.os_version);
    
    return 0;
}
```

编译时链接：

```bash
gcc -o myapp myapp.c console_detect.c
```

## 清理

```bash
make clean
```

## 许可证

[MIT License](LICENSE)
