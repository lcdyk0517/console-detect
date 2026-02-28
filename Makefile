# console_detect Makefile
# Linux ARM64 设备检测工具 (ArkOS4Clone)

CC = gcc
CROSS_COMPILE = /opt/toolchains/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

# 使用交叉编译时
ifeq ($(CROSS),1)
    CC = $(CROSS_COMPILE)gcc
endif

CFLAGS = -Wall -Wextra -O2
LDFLAGS = 

TARGET = console_detect
SRC = console_detect.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean install arm64 native

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 本地编译 (x86_64)
native:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# ARM64 交叉编译
arm64:
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $(TARGET) $(SRC)

# 安装
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

# 清理
clean:
	rm -f $(TARGET) $(OBJ)

# 静态分析
check:
	cppcheck --enable=all $(SRC)

# 显示帮助
help:
	@echo "可用目标:"
	@echo "  all     - 编译程序 (默认)"
	@echo "  native  - 本地编译 (x86_64)"
	@echo "  arm64   - ARM64 交叉编译"
	@echo "  install - 安装到 /usr/local/bin/"
	@echo "  clean   - 清理编译文件"
	@echo "  check   - 静态代码分析"
	@echo ""
	@echo "使用方式:"
	@echo "  make           - 本地编译"
	@echo "  make CROSS=1   - 使用交叉编译器"