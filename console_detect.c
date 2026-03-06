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

/* 屏幕分辨率映射 */
typedef struct {
    const char *resolution_str;
    int width;
    int height;
} ResolutionMap;

static ResolutionMap resolution_map[] = {
    {"480p",   640,  480},
    {"540p",   720,  540},
    {"720p",   720, 720},
    {"768p",   1024, 768},
    {"800p480", 800, 480},
    {"854p480", 854, 480},
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
} DeviceInfo;

/* 设备配置表 */
typedef struct {
    const char *name;
    const char *resolution_str;
    const char *joystick_type;  /* single, dual, none */
    const char *hotkey_type;    /* select, happy5 */
    int rotation;
    const char *led_type;       /* mcu_led, gpio, ws2812, unsupported */
} DeviceConfig;

/* 预定义设备配置 */
static DeviceConfig device_configs[] = {
    {"mymini",    "480p",   "single", "select", 0,    "gpio"},
    {"mini40",    "480p",   "single", "select", 0,    "gpio"},
    {"xf35h",     "480p",   "dual",   "select", 0,    "mcu_led"},
    {"r36pro",    "480p",   "dual",   "happy5", 0,    "unsupported"},
    {"r36max",    "720p",   "dual",   "happy5", 0,    "unsupported"},
    {"xf40h",     "720p",   "dual",   "select", 0,    "mcu_led"},
    {"dc40v",     "720p",   "dual",   "happy5", 0,    "ws2812"},
    {"dc35v",     "480p",   "dual",   "happy5", 0,    "ws2812"},
    {"r36max2",   "768p",   "dual",   "happy5", 0,    "ws2812"},
    {"r36h",      "480p",   "dual",   "select", 0,    "unsupported"},
    {"r36splus",  "720p",   "dual",   "happy5", 0,    "unsupported"},
    {"r46h",      "768p",   "dual",   "select", 0,    "unsupported"},
    {"r40xx",     "768p",   "dual",   "happy5", 0,    "unsupported"},
    {"hg36",      "480p",   "dual",   "happy5", 0,    "unsupported"},
    {"rx6h",      "480p",   "dual",   "select", 0,    "unsupported"},
    {"k36s",      "480p",   "single", "happy5", 0,    "mcu_led"},
    {"r36tmax",   "720p",   "dual",   "happy5", 0,    "mcu_led"},
    {"t16max",    "720p",   "dual",   "happy5", 0,    "unsupported"},
    {"r36ultra",  "720p",   "dual",   "happy5", 0,    "gpio"},
    {"xgb36",     "480p",   "single", "happy5", 0,    "gpio"},
    {"a10mini",   "480p",   "none",   "happy5", 0,    "unsupported"},
    {"a10miniv2", "540p",   "none",   "happy5", 180,  "unsupported"},
    {"g350",      "480p",   "dual",   "happy5", 0,    "unsupported"},
    {"u8",        "800p480","dual",   "happy5", 270,  "unsupported"},
    {"dr28s",     "480p",   "none",   "happy5", 270,  "unsupported"},
    {"d007",      "480p",   "dual",   "select", 0,    "dual-gpio"},
    {"r50s",      "854p480","dual",   "happy5", 270,  "unsupported"},
    {"rgb20s",    "480p",   "dual",   "happy5", 0,    "unsupported"},
    {"xf28",      "480p",   "single", "select", 90,   "ws2812"},
    {"r33s",      "480p",   "none",   "select", 0,    "unsupported"},
    {"r36s",      "480p",   "dual",   "happy5", 0,    "unsupported"},
    {NULL, NULL, NULL, NULL, 0, NULL}
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
    {"rk3326-r36pro-linux.dtb",       "r36pro"},
    {"rk3326-r36max-linux.dtb",       "r36max"},
    {"rk3326-xf40h-linux.dtb",        "xf40h"},
    {"rk3326-dc40v-linux.dtb",        "dc40v"},
    {"rk3326-dc35v-linux.dtb",        "dc35v"},
    {"rk3326-r36max2-linux.dtb",      "r36max2"},
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
    {"rk3326-xgb36-linux.dtb",        "xgb36"},
    {"rk3326-a10mini-linux.dtb",      "a10mini"},
    {"rk3326-a10mini-v2-linux.dtb",   "a10miniv2"},
    {"rk3326-g350-linux.dtb",         "g350"},
    {"rk3326-u8-linux.dtb",           "u8"},
    {"rk3326-u8-v2-linux.dtb",        "u8"},
    {"rk3326-dr28s-linux.dtb",        "dr28s"},
    {"rk3326-d007-linux.dtb",         "d007"},
    {"rk3326-r50s-linux.dtb",         "r50s"},
    {"rk3326-rgb20s-linux.dtb",       "rgb20s"},
    {"rk3326-xf28-linux.dtb",         "xf28"},
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
    printf("  \"led_type\": \"%s\"\n", info->led_type);
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
    printf("  -b, --bootini     仅输出 boot.ini 检测的设备名称\n");
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
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--bootini") == 0) {
            single_output = 7;
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
