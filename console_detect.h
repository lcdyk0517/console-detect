/*
 * console_detect.h - 设备检测库头文件 (ArkOS4Clone)
 */

#ifndef CONSOLE_DETECT_H
#define CONSOLE_DETECT_H

#ifdef __cplusplus
extern "C" {
#endif

/* 设备信息结构体 */
typedef struct {
    char device_name[64];
    int screen_width;
    int screen_height;
    int joystick_count;     /* 0: none, 1: single, 2: dual */
    const char *hotkey_type;
    int rotation;           /* 屏幕旋转角度 */
    const char *led_type;   /* mcu_led, gpio, ws2812, unsupported */
} DeviceInfo;

/* 获取设备信息 */
int get_device_info(DeviceInfo *info);

/* 打印设备信息 (人类可读格式) */
void print_device_info(const DeviceInfo *info);

/* 打印设备信息 (JSON格式) */
void print_device_info_json(const DeviceInfo *info);

/* 打印设备信息 (Shell变量格式) */
void print_device_info_shell(const DeviceInfo *info);

#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_DETECT_H */