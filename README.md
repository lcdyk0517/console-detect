# console_detect - Linux ARM64 设备检测工具

用于 ArkOS4Clone 项目的设备信息检测工具，可自动识别手持游戏设备并输出其硬件配置信息。

## 功能特点

- 自动检测设备型号
- 获取屏幕分辨率
- 识别摇杆配置（单摇杆/双摇杆/无摇杆）
- 获取热键类型
- 读取屏幕旋转角度
- 识别 LED 类型

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
  "led_type": "unsupported"
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
```

## 配置文件

程序从 `/boot/.console` 文件读取设备名称。如果该文件不存在，会自动从 `/boot/boot.ini` 解析 DTB 文件名并推断设备型号，然后将结果写入 `.console` 文件。

### 手动指定设备

可以手动创建 `/boot/.console` 文件：

```bash
echo "r36s" > /boot/.console
```

## 支持的设备

| 设备代号 | 分辨率 | 摇杆 | 热键类型 | LED 类型 |
|----------|--------|------|----------|----------|
| mymini | 640x480 | 单摇杆 | select | gpio |
| mini40 | 640x480 | 单摇杆 | select | gpio |
| xf35h | 640x480 | 双摇杆 | select | mcu_led |
| r36pro | 640x480 | 双摇杆 | happy5 | unsupported |
| r36max | 720x720 | 双摇杆 | happy5 | unsupported |
| xf40h | 720x720 | 双摇杆 | select | mcu_led |
| dc40v | 720x720 | 双摇杆 | happy5 | ws2812 |
| r36max2 | 1024x768 | 双摇杆 | happy5 | ws2812 |
| r36h | 640x480 | 双摇杆 | select | unsupported |
| r36splus | 720x720 | 双摇杆 | happy5 | unsupported |
| r46h | 1024x768 | 双摇杆 | select | unsupported |
| r40xx | 1024x768 | 双摇杆 | happy5 | unsupported |
| hg36 | 640x480 | 双摇杆 | happy5 | unsupported |
| rx6h | 640x480 | 双摇杆 | select | unsupported |
| k36s | 640x480 | 单摇杆 | happy5 | mcu_led |
| r36tmax | 720x720 | 双摇杆 | happy5 | mcu_led |
| t16max | 720x720 | 双摇杆 | happy5 | unsupported |
| r36ultra | 720x720 | 双摇杆 | happy5 | gpio |
| xgb36 | 640x480 | 单摇杆 | happy5 | gpio |
| a10mini | 640x480 | 无摇杆 | happy5 | unsupported |
| a10miniv2 | 720x540 | 无摇杆 | happy5 | unsupported |
| g350 | 640x480 | 双摇杆 | happy5 | unsupported |
| u8 | 800x480 | 双摇杆 | happy5 | unsupported |
| dr28s | 640x480 | 无摇杆 | happy5 | unsupported |
| d007 | 640x480 | 双摇杆 | select | dual-gpio |
| r50s | 854x480 | 双摇杆 | happy5 | unsupported |
| rgb20s | 640x480 | 双摇杆 | happy5 | unsupported |
| xf28 | 640x480 | 单摇杆 | select | ws2812 |
| r36s | 640x480 | 双摇杆 | happy5 | unsupported |
| r33s | 640x480 | 无摇杆 | select | unsupported |

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
    
    printf("设备: %s, 分辨率: %dx%d\n", 
           info.device_name, 
           info.screen_width, 
           info.screen_height);
    
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
