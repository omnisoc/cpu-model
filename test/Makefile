# RISC-V 交叉编译工具链前缀
RISCV_PREFIX = riscv64-unknown-elf-
AS = $(RISCV_PREFIX)as
LD = $(RISCV_PREFIX)ld
OBJCOPY = $(RISCV_PREFIX)objcopy

# 输入文件
SRC = test.s
OBJ = test.o
ELF = test.elf
BIN = test.bin

# 链接地址，需与模拟器加载地址一致
LINK_ADDR = 0x10000

# 默认目标：生成二进制文件
all: $(BIN)

# 1. 汇编：将 .s 转换为 .o
$(OBJ): $(SRC)
	@echo "[AS] 汇编 $< -> $@"
	$(AS) -o $@ $<

# 2. 链接：将 .o 转换为 .elf，指定代码段起始地址
$(ELF): $(OBJ)
	@echo "[LD] 链接 $< -> $@ (地址: $(LINK_ADDR))"
	$(LD) -Ttext=$(LINK_ADDR) -o $@ $<

# 3. 提取：将 .elf 转换为纯二进制 .bin
$(BIN): $(ELF)
	@echo "[OBJCOPY] 生成二进制镜像 $@"
	$(OBJCOPY) -O binary $< $@

# 清理
clean:
	@echo "清理测试文件..."
	rm -f $(OBJ) $(ELF) $(BIN)

.PHONY: all clean

