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
  "otg_type": "auto"
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
```

## 配置文件

程序从 `/boot/.console` 文件读取设备名称。如果该文件不存在，会自动从 `/boot/boot.ini` 解析 DTB 文件名并推断设备型号，然后将结果写入 `.console` 文件。

### 手动指定设备

可以手动创建 `/boot/.console` 文件：

```bash
echo "r36s" > /boot/.console
```

## 支持的设备

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | LED 类型 | OTG 类型 |
|----------|--------|------|----------|----------|----------|
| mymini | 480p | 单摇杆 | select | gpio | auto |
| mini40 | 720p | 单摇杆 | select | gpio | auto |
| xf35h | 480p | 双摇杆 | select | mcu_led | auto |
| rf35h | 480p | 双摇杆 | select | mcu_led | auto |
| r36pro | 480p | 双摇杆 | happy5 | unsupported | auto |
| r36max | 720p | 双摇杆 | happy5 | unsupported | auto |
| xf40h | 720p | 双摇杆 | select | mcu_led | auto |
| rf40h | 720p | 双摇杆 | select | mcu_led | auto |
| dc40v | 720p | 双摇杆 | happy5 | ws2812 | auto |
| dc35v | 480p | 双摇杆 | happy5 | ws2812 | auto |
| r36max2 | 768p | 双摇杆 | happy5 | ws2812 | manual |
| rf45v | 768p | 双摇杆 | happy5 | ws2812 | manual |
| xf45v | 768p | 双摇杆 | happy5 | ws2812 | manual |
| dc45v | 768p | 双摇杆 | happy5 | ws2812 | manual |
| r36h | 480p | 双摇杆 | select | unsupported | auto |
| r36splus | 720p | 双摇杆 | happy5 | unsupported | auto |
| r46h | 768p | 双摇杆 | select | unsupported | auto |
| r40xx | 768p | 双摇杆 | happy5 | unsupported | auto |
| hg36 | 480p | 双摇杆 | happy5 | unsupported | auto |
| rx6h | 480p | 双摇杆 | select | unsupported | auto |
| k36s | 480p | 单摇杆 | happy5 | mcu_led | auto |
| r36tmax | 720p | 双摇杆 | happy5 | mcu_led | auto |
| t16max | 720p | 双摇杆 | happy5 | unsupported | auto |
| r36ultra | 720p | 双摇杆 | happy5 | r36ultra | auto |
| r36ultrax | 768p | 双摇杆 | happy5 | ws2812 | auto |
| xgb36 | 480p | 单摇杆 | happy5 | gpio | auto |
| a10mini | 480p | 无摇杆 | happy5 | unsupported | auto |
| a10miniv4 | 540p | 无摇杆 | happy5 | unsupported | auto |
| g350 | 480p | 双摇杆 | happy5 | unsupported | auto |
| u8 | 800p480 | 双摇杆 | happy5 | unsupported | auto |
| dr28s | 480p | 无摇杆 | happy5 | unsupported | auto |
| d007 | 480p | 双摇杆 | select | dual-gpio | auto |
| r50s | 854p480 | 双摇杆 | happy5 | unsupported | auto |
| r50h | 720p1280 | 双摇杆 | happy5 | unsupported | auto |
| rgb20s | 480p | 双摇杆 | happy5 | unsupported | auto |
| xf28 | 480p | 单摇杆 | select | ws2812 | auto |
| r33s | 480p | 无摇杆 | select | unsupported | auto |
| xu10 | 480p | 无摇杆 | happy5 | unsupported | auto |
| r40s | 800p480 | 双摇杆 | happy5 | unsupported | auto |
| rgb10max1 | 854p480 | 双摇杆 | happy5 | unsupported | auto |
| rgb10 | 320p | 单摇杆 | select | unsupported | auto |
| r36s | 480p | 双摇杆 | happy5 | unsupported | auto |

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
    
    printf("设备: %s, 分辨率: %dx%d, OTG: %s\n", 
           info.device_name, 
           info.screen_width, 
           info.screen_height,
           info.otg_type);
    
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
