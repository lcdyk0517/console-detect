/*
 * console_detect.c - Linux ARM64 设备检测工具 (ArkOS4Clone)
 * 读取 /boot/.console 文件并解析设备配置信息
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256
#define MAX_DEVICES 64
#define CONFIG_FILE "/boot/.console"
#define BOOT_INI_FILE "/boot/boot.ini"
#define OS_RELEASE_FILE "/etc/os-release"

/* 屏幕分辨率映射 */
typedef struct {
    const char *resolution_str;
    int width;
    int height;
} ResolutionMap;

static ResolutionMap resolution_map[] = {
    {"320p",   480,  320},
    {"480p",   640,  480},
    {"540p",   720,  540},
    {"720p",   720, 720},
    {"768p",   1024, 768},
    {"800p480", 800, 480},
    {"854p480", 854, 480},
    {"720p1280", 1280, 720},
    {NULL, 0, 0}
};

/* 设备信息结构体 */
typedef struct {
    char device_name[64];
    int screen_width;
    int screen_height;
    int joystick_count;     /* -1: none, 0: single, 1: dual */
    const char *hotkey_type;
    int rotation;           /* 屏幕旋转角度 */
    const char *led_type;   /* mcu_led, gpio, ws2812, unsupported */
    const char *otg_type;   /* auto, manual */
    char os_version[128];   /* 系统版本 */
} DeviceInfo;

/* 设备配置表 */
typedef struct {
    const char *name;
    const char *resolution_str;
    const char *joystick_type;  /* single, dual, none */
    const char *hotkey_type;    /* select, happy5 */
    int rotation;
    const char *led_type;       /* mcu_led, gpio, ws2812, unsupported */
    const char *otg_type;       /* auto, manual */
} DeviceConfig;

/* 预定义设备配置 */
static DeviceConfig device_configs[] = {
    {"mymini",    "480p",   "single", "select", 0,    "gpio",       "auto"},
    {"mini40",    "720p",   "single", "select", 0,    "gpio",       "auto"},
    {"xf35h",     "480p",   "dual",   "select", 0,    "mcu_led",    "auto"},
    {"rf35h",     "480p",   "dual",   "select", 0,    "mcu_led",    "auto"},
    {"r36pro",    "480p",   "dual",   "happy5", 0,    "unsupported","auto"},
    {"r36max",    "720p",   "dual",   "happy5", 0,    "unsupported","auto"},
    {"xf40h",     "720p",   "dual",   "select", 0,    "mcu_led",    "auto"},
    {"rf40h",     "720p",   "dual",   "select", 0,    "mcu_led",    "auto"},
    {"rf55h",     "720p1280","dual",  "happy5", 90,   "ws2812",     "manual"},
    {"dc40v",     "720p",   "dual",   "happy5", 0,    "ws2812",     "auto"},
    {"dc35v",     "480p",   "dual",   "happy5", 0,    "ws2812",     "auto"},
    {"r36max2",   "768p",   "dual",   "happy5", 0,    "ws2812",     "manual"},
    {"rf45v",     "768p",   "dual",   "happy5", 0,    "ws2812",     "manual"},
    {"xf45v",     "768p",   "dual",   "happy5", 0,    "ws2812",     "manual"},
    {"dc45v",     "768p",   "dual",   "happy5", 0,    "ws2812",     "manual"},
    {"r36h",      "480p",   "dual",   "select", 0,    "unsupported","auto"},
    {"r36splus",  "720p",   "dual",   "happy5", 0,    "unsupported","auto"},
    {"r46h",      "768p",   "dual",   "select", 0,    "unsupported","auto"},
    {"r40xx",     "768p",   "dual",   "happy5", 0,    "unsupported","auto"},
    {"hg36",      "480p",   "dual",   "happy5", 0,    "unsupported","auto"},
    {"rx6h",      "480p",   "dual",   "select", 0,    "unsupported","auto"},
    {"k36s",      "480p",   "single", "happy5", 0,    "mcu_led",    "auto"},
    {"r36tmax",   "720p",   "dual",   "happy5", 0,    "mcu_led",    "auto"},
    {"t16max",    "720p",   "dual",   "happy5", 0,    "unsupported","auto"},
    {"r36ultra",  "720p",   "dual",   "happy5", 0,    "r36ultra",   "auto"},
    {"r36ultrax", "768p",   "dual",   "happy5", 0,    "ws2812",     "auto"},
    {"xgb36",     "480p",   "single", "happy5", 0,    "gpio",       "auto"},
    {"a10mini",   "480p",   "none",   "happy5", 0,    "unsupported","auto"},
    {"a10miniv4", "540p",   "none",   "happy5", 180,  "unsupported","auto"},
    {"g350",      "480p",   "dual",   "happy5", 0,    "unsupported","auto"},
    {"u8",        "800p480","dual",   "happy5", 270,  "unsupported","auto"},
    {"dr28s",     "480p",   "none",   "happy5", 270,  "unsupported","auto"},
    {"d007",      "480p",   "dual",   "select", 0,    "dual-gpio",  "auto"},
    {"r50s",      "854p480","dual",   "happy5", 270,  "unsupported","auto"},
    {"r50h",      "720p1280","dual",  "happy5", 270,  "unsupported","auto"},
    {"rgb20s",    "480p",   "dual",   "happy5", 0,    "unsupported","auto"},
    {"xf28",      "480p",   "single", "select", 90,   "ws2812",     "auto"},
    {"r33s",      "480p",   "none",   "select", 0,    "unsupported","auto"},
    {"xu10",      "480p",   "none",   "happy5", 0,    "unsupported","auto"},
    {"r40s",      "800p480","dual",   "happy5", 270,  "unsupported","auto"},
    {"rgb10max1", "854p480","dual",   "happy5", 270,  "unsupported","auto"},
    {"rgb10",     "320p",   "single", "select", 270,  "unsupported","auto"},
    {"h7",        "768p",   "single", "happy5", 0,    "ws2812",     "auto"},
    {"go2",       "768p",   "single", "happy5", 0,    "ws2812",     "auto"},
    {"r36s",      "480p",   "dual",   "happy5", 0,    "unsupported","auto"},
    {NULL, NULL, NULL, NULL, 0, NULL, NULL}
};

/* DTB 到设备名称映射表 */
typedef struct {
    const char *dtb_name;
    const char *device_name;
} DtbMap;

static DtbMap dtb_mapping[] = {
    {"rk3326-mymini-linux.dtb",       "mymini"},
    {"rk3326-mini40-linux.dtb",       "mini40"},
    {"rk3326-xf35h-linux.dtb",        "xf35h"},
    {"rk3326-rf35h-linux.dtb",        "rf35h"},
    {"rk3326-r36pro-linux.dtb",       "r36pro"},
    {"rk3326-r36max-linux.dtb",       "r36max"},
    {"rk3326-r36max-without-amp-linux.dtb",       "r36max"},
    {"rk3326-xf40h-linux.dtb",        "xf40h"},
    {"rk3326-rf40h-linux.dtb",        "rf40h"},
    {"rk3326-rf55h-linux.dtb",        "rf55h"},
    {"rk3326-dc40v-linux.dtb",        "dc40v"},
    {"rk3326-dc35v-linux.dtb",        "dc35v"},
    {"rk3326-r36max2-linux.dtb",      "r36max2"},
    {"rk3326-rf45v-linux.dtb",        "rf45v"},
    {"rk3326-xf45v-linux.dtb",        "xf45v"},
    {"rk3326-dc45v-linux.dtb",        "dc45v"},
    {"rk3326-r36h-linux.dtb",         "r36h"},
    {"rk3326-r36splus-linux.dtb",     "r36splus"},
    {"rk3326-r46h-linux.dtb",         "r46h"},
    {"rk3326-r33s-linux.dtb",         "r33s"},
    {"rk3326-r40xx-linux.dtb",        "r40xx"},
    {"rk3326-hg36-linux.dtb",         "hg36"},
    {"rk3326-rx6h-linux.dtb",         "rx6h"},
    {"rk3326-k36s-linux.dtb",         "k36s"},
    {"rk3326-r36tmax-linux.dtb",      "r36tmax"},
    {"rk3326-t16max-linux.dtb",       "t16max"},
    {"rk3326-r36ultra-linux.dtb",     "r36ultra"},
    {"rk3326-r36ultrax-linux.dtb",    "r36ultrax"},
    {"rk3326-xgb36-linux.dtb",        "xgb36"},
    {"rk3326-a10mini-linux.dtb",      "a10mini"},
    {"rk3326-a10mini-v4-linux.dtb",   "a10miniv4"},
    {"rk3326-g350-linux.dtb",         "g350"},
    {"rk3326-u8-linux.dtb",           "u8"},
    {"rk3326-u8-v2-linux.dtb",        "u8"},
    {"rk3326-dr28s-linux.dtb",        "dr28s"},
    {"rk3326-d007-linux.dtb",         "d007"},
    {"rk3326-r50s-linux.dtb",         "r50s"},
    {"rk3326-r50h-linux.dtb",         "r50h"},
    {"rk3326-rgb20s-linux.dtb",       "rgb20s"},
    {"rk3326-xu10-linux.dtb",         "xu10"},
    {"rk3326-r40s-linux.dtb",         "r40s"},
    {"rk3326-xf28-linux.dtb",         "xf28"},
    {"rk3326-rgb10max1-linux.dtb",    "rgb10max1"},
    {"rk3326-rgb10-linux.dtb",        "rgb10"},
    {"rk3326-h7-linux.dtb",           "h7"},
    {"rk3326-go2-linux.dtb",          "go2"},
    {NULL, NULL}
};

/* 去除字符串两端空白 */
static char *trim(char *str) {
    char *end;
    
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    
    return str;
}

/* 解析分辨率字符串 */
static void parse_resolution(const char *res_str, int *width, int *height) {
    ResolutionMap *map = resolution_map;
    
    while (map->resolution_str != NULL) {
        if (strcmp(res_str, map->resolution_str) == 0) {
            *width = map->width;
            *height = map->height;
            return;
        }
        map++;
    }
    
    /* 默认值 */
    *width = 640;
    *height = 480;
}

/* 解析摇杆类型 */
static int parse_joystick(const char *joystick_type) {
    if (strcmp(joystick_type, "single") == 0) return 1;
    if (strcmp(joystick_type, "dual") == 0) return 2;
    if (strcmp(joystick_type, "none") == 0) return 0;
    return 1;  /* 默认单摇杆 */
}

/* 从配置表中查找设备 */
static DeviceConfig *find_device_config(const char *device_name) {
    DeviceConfig *config = device_configs;
    
    while (config->name != NULL) {
        if (strcmp(config->name, device_name) == 0) {
            return config;
        }
        config++;
    }
    
    return NULL;
}

/* 从 DTB 映射表中查找设备名称 */
static const char *find_device_by_dtb(const char *dtb_name) {
    DtbMap *map = dtb_mapping;
    
    while (map->dtb_name != NULL) {
        if (strcmp(map->dtb_name, dtb_name) == 0) {
            return map->device_name;
        }
        map++;
    }
    
    return NULL;
}

/* 从 boot.ini 读取 dtb 并获取设备名称 */
static int read_boot_ini(char *device_name, size_t max_len) {
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    char *dtb_start, *dtb_end;
    char dtb_name[128];
    
    fp = fopen(BOOT_INI_FILE, "r");
    if (fp == NULL) {
        return -1;
    }
    
    device_name[0] = '\0';
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        /* 查找包含 dtb_loadaddr 的行 */
        if (strstr(line, "${dtb_loadaddr}") != NULL) {
            dtb_start = strstr(line, "${dtb_loadaddr}");
            if (dtb_start != NULL) {
                dtb_start += strlen("${dtb_loadaddr}");
                /* 跳过空格 */
                while (*dtb_start == ' ' || *dtb_start == '\t') dtb_start++;
                
                /* 提取 dtb 文件名 */
                dtb_end = dtb_start;
                while (*dtb_end != '\0' && *dtb_end != '\n' && *dtb_end != '\r' && 
                       *dtb_end != ' ' && *dtb_end != '\t') {
                    dtb_end++;
                }
                
                int len = dtb_end - dtb_start;
                if (len > 0 && len < (int)sizeof(dtb_name)) {
                    strncpy(dtb_name, dtb_start, len);
                    dtb_name[len] = '\0';
                    
                    /* 查找映射，未找到则使用默认值 r36s */
                    const char *dev_name = find_device_by_dtb(dtb_name);
                    if (dev_name == NULL) {
                        dev_name = "r36s";
                    }
                    strncpy(device_name, dev_name, max_len - 1);
                    device_name[max_len - 1] = '\0';
                    break;
                }
            }
        }
    }
    
    fclose(fp);
    
    return (device_name[0] != '\0') ? 0 : -1;
}

/* 写入设备名称到 .console 文件 */
static int write_console_file(const char *device_name) {
    FILE *fp;
    
    fp = fopen(CONFIG_FILE, "w");
    if (fp == NULL) {
        return -1;
    }
    
    fprintf(fp, "%s\n", device_name);
    fclose(fp);
    
    return 0;
}

/* 读取 .console 文件并获取设备名称 */
static int read_console_file(char *device_name, size_t max_len) {
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    
    fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        /* .console 不存在，尝试从 boot.ini 读取 */
        if (read_boot_ini(device_name, max_len) == 0) {
            /* 写入 .console 文件 */
            write_console_file(device_name);
            return 0;
        }
        fprintf(stderr, "无法打开配置文件: %s\n", CONFIG_FILE);
        return -1;
    }
    
    device_name[0] = '\0';
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        /* 跳过注释和空行 */
        char *trimmed = trim(line);
        if (*trimmed == '#' || *trimmed == '\0') {
            continue;
        }
        
        /* 直接读取设备名称 */
        strncpy(device_name, trimmed, max_len - 1);
        device_name[max_len - 1] = '\0';
        break;
    }
    
    fclose(fp);
    
    if (device_name[0] == '\0') {
        fprintf(stderr, "未在配置文件中找到设备信息\n");
        return -1;
    }
    
    return 0;
}

/* 读取系统版本 */
static int read_os_version(char *os_version, size_t max_len) {
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    char *value_start, *value_end;
    
    fp = fopen(OS_RELEASE_FILE, "r");
    if (fp == NULL) {
        strncpy(os_version, "Unknown", max_len - 1);
        os_version[max_len - 1] = '\0';
        return -1;
    }
    
    os_version[0] = '\0';
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *trimmed = trim(line);
        
        /* 查找 PRETTY_NAME 字段 */
        if (strncmp(trimmed, "PRETTY_NAME=", 12) == 0) {
            value_start = trimmed + 12;
            
            /* 跳过开头的引号 */
            if (*value_start == '"') {
                value_start++;
            }
            
            /* 查找结尾的引号或行尾 */
            value_end = value_start;
            while (*value_end != '\0' && *value_end != '"' && *value_end != '\n' && *value_end != '\r') {
                value_end++;
            }
            
            int len = value_end - value_start;
            if (len > 0 && len < (int)max_len) {
                strncpy(os_version, value_start, len);
                os_version[len] = '\0';
                
                /* 如果是 Ubuntu 19.10 输出 ArkOS4Clone，Debian 13 输出 dArkOS4Clone，否则输出 NULL */
                if (strcmp(os_version, "Ubuntu 19.10") == 0) {
                    strncpy(os_version, "ArkOS4Clone", max_len - 1);
                } else if (strcmp(os_version, "Debian GNU/Linux 13 (trixie)") == 0) {
                    strncpy(os_version, "dArkOS4Clone", max_len - 1);
                } else {
                    strncpy(os_version, "NULL", max_len - 1);
                }
                os_version[max_len - 1] = '\0';
                break;
            }
        }
    }
    
    fclose(fp);
    
    if (os_version[0] == '\0') {
        strncpy(os_version, "Unknown", max_len - 1);
        os_version[max_len - 1] = '\0';
        return -1;
    }
    
    return 0;
}

/* 获取设备信息 */
int get_device_info(DeviceInfo *info) {
    char device_name[64];
    DeviceConfig *config;
    
    if (read_console_file(device_name, sizeof(device_name)) != 0) {
        return -1;
    }
    
    config = find_device_config(device_name);
    if (config == NULL) {
        fprintf(stderr, "未知设备: %s\n", device_name);
        return -1;
    }
    
    /* 填充设备信息 */
    strncpy(info->device_name, config->name, sizeof(info->device_name) - 1);
    info->device_name[sizeof(info->device_name) - 1] = '\0';
    
    parse_resolution(config->resolution_str, &info->screen_width, &info->screen_height);
    info->joystick_count = parse_joystick(config->joystick_type);
    info->hotkey_type = config->hotkey_type;
    info->rotation = config->rotation;
    info->led_type = config->led_type;
    info->otg_type = config->otg_type;
    
    /* 读取系统版本 */
    read_os_version(info->os_version, sizeof(info->os_version));
    
    return 0;
}

/* 打印设备信息 */
void print_device_info(const DeviceInfo *info) {
    printf("========== 设备信息 ==========\n");
    printf("设备名称:     %s\n", info->device_name);
    printf("屏幕分辨率:   %d x %d\n", info->screen_width, info->screen_height);
    printf("摇杆数量:     %d\n", info->joystick_count);
    printf("热键类型:     %s\n", info->hotkey_type);
    printf("屏幕旋转:     %d 度\n", info->rotation);
    printf("LED类型:      %s\n", info->led_type);
    printf("OTG类型:      %s\n", info->otg_type);
    printf("系统版本:     %s\n", info->os_version);
    printf("==============================\n");
}

/* JSON 格式输出 */
void print_device_info_json(const DeviceInfo *info) {
    printf("{\n");
    printf("  \"device_name\": \"%s\",\n", info->device_name);
    printf("  \"screen_width\": %d,\n", info->screen_width);
    printf("  \"screen_height\": %d,\n", info->screen_height);
    printf("  \"joystick_count\": %d,\n", info->joystick_count);
    printf("  \"hotkey_type\": \"%s\",\n", info->hotkey_type);
    printf("  \"rotation\": %d,\n", info->rotation);
    printf("  \"led_type\": \"%s\",\n", info->led_type);
    printf("  \"otg_type\": \"%s\",\n", info->otg_type);
    printf("  \"os_version\": \"%s\"\n", info->os_version);
    printf("}\n");
}

/* Shell 变量格式输出 */
void print_device_info_shell(const DeviceInfo *info) {
    printf("DEVICE_NAME=%s\n", info->device_name);
    printf("SCREEN_WIDTH=%d\n", info->screen_width);
    printf("SCREEN_HEIGHT=%d\n", info->screen_height);
    printf("JOYSTICK_COUNT=%d\n", info->joystick_count);
    printf("HOTKEY_TYPE=%s\n", info->hotkey_type);
    printf("SCREEN_ROTATION=%d\n", info->rotation);
    printf("LED_TYPE=%s\n", info->led_type);
    printf("OTG_TYPE=%s\n", info->otg_type);
    printf("OS_VERSION=%s\n", info->os_version);
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项]\n", program_name);
    printf("\n选项:\n");
    printf("  -h, --help     显示帮助信息\n");
    printf("  -j, --json     JSON 格式输出\n");
    printf("  -s, --shell    Shell 变量格式输出\n");
    printf("  -n, --name     仅输出设备名称\n");
    printf("  -r, --resolution  仅输出分辨率 (格式: 宽x高)\n");
    printf("  -k, --joystick    仅输出摇杆数量\n");
    printf("  -t, --hotkey      仅输出热键类型\n");
    printf("  -o, --rotation    仅输出屏幕旋转角度\n");
    printf("  -l, --led         仅输出LED类型\n");
    printf("  -O, --otg         仅输出OTG类型\n");
    printf("  -b, --bootini     仅输出 boot.ini 检测的设备名称\n");
    printf("  -V, --version     仅输出系统版本\n");
}

int main(int argc, char *argv[]) {
    DeviceInfo info;
    int output_format = 0;  /* 0: 普通格式, 1: JSON, 2: Shell */
    int single_output = 0;  /* 单项输出模式 */
    char bootini_device[64] = "";
    
    /* 解析命令行参数 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--json") == 0) {
            output_format = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--shell") == 0) {
            output_format = 2;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) {
            single_output = 1;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--resolution") == 0) {
            single_output = 2;
        } else if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--joystick") == 0) {
            single_output = 3;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--hotkey") == 0) {
            single_output = 4;
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--rotation") == 0) {
            single_output = 5;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--led") == 0) {
            single_output = 6;
        } else if (strcmp(argv[i], "-O") == 0 || strcmp(argv[i], "--otg") == 0) {
            single_output = 8;
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--bootini") == 0) {
            single_output = 7;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            single_output = 9;
        }
    }
    
    /* 如果是 -b 选项，直接从 boot.ini 读取 */
    if (single_output == 7) {
        if (read_boot_ini(bootini_device, sizeof(bootini_device)) == 0) {
            printf("%s\n", bootini_device);
            return 0;
        } else {
            fprintf(stderr, "无法从 boot.ini 读取设备信息\n");
            return 1;
        }
    }
    
    /* 获取设备信息 */
    if (get_device_info(&info) != 0) {
        return 1;
    }
    
    /* 根据模式输出 */
    if (single_output > 0) {
        switch (single_output) {
            case 1: printf("%s\n", info.device_name); break;
            case 2: printf("%dx%d\n", info.screen_width, info.screen_height); break;
            case 3: printf("%d\n", info.joystick_count); break;
            case 4: printf("%s\n", info.hotkey_type); break;
            case 5: printf("%d\n", info.rotation); break;
            case 6: printf("%s\n", info.led_type); break;
            case 8: printf("%s\n", info.otg_type); break;
            case 9: printf("%s\n", info.os_version); break;
        }
    } else {
        switch (output_format) {
            case 1: print_device_info_json(&info); break;
            case 2: print_device_info_shell(&info); break;
            default: print_device_info(&info); break;
        }
    }
    
    return 0;
}
