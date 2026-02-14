#!/bin/bash

# 定义颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 项目根目录
ROOT_DIR=$(cd "$(dirname "$0")";pwd)
BUILD_DIR="${ROOT_DIR}/build"
BIN_DIR="${ROOT_DIR}/bin"
TEST_DIR="${ROOT_DIR}/test"

# 清理函数
clean() {
    echo -e "${YELLOW}正在清理构建产物...${NC}"
    if [ -d "$BUILD_DIR" ]; then rm -rf "$BUILD_DIR"; fi
    if [ -d "$BIN_DIR" ]; then rm -rf "$BIN_DIR"; fi
    # 清理 test 目录下的 elf 文件
    rm -f "${TEST_DIR}"/*.elf
    echo -e "${GREEN}清理完成！${NC}"
}

# 编译函数
build() {
    echo -e "${YELLOW}[1/2] 正在编译模拟器...${NC}"
    if [ ! -d "$BUILD_DIR" ]; then mkdir -p "$BUILD_DIR"; fi
    cd "$BUILD_DIR" || exit 1
    if ! cmake .. > /dev/null; then
        echo -e "${RED}CMake 配置失败！${NC}"
        cmake ..
        exit 1
    fi
    if make; then
        if [ -f "${BIN_DIR}/riscv_emu" ]; then
            echo -e "${GREEN}模拟器编译成功: ${BIN_DIR}/riscv_emu${NC}"
        else
            echo -e "${RED}未生成可执行文件！${NC}"
            exit 1
        fi
    else
        echo -e "${RED}编译失败！${NC}"
        exit 1
    fi
}

# 测试函数
test() {
    echo -e "${YELLOW}[2/2] 正在运行测试...${NC}"
    
    local EMULATOR="${BIN_DIR}/riscv_emu"
    local TEST_SRC="${TEST_DIR}/test.s"
    # 修改：直接生成并使用 .elf 文件
    local TEST_ELF="${BUILD_DIR}/test.elf" 

    if [ ! -f "$EMULATOR" ]; then
        echo -e "${RED}错误: 找不到模拟器，请先运行 './run.sh build'${NC}"
        exit 1
    fi

    if [ ! -f "$TEST_SRC" ]; then
        echo -e "${RED}错误: 找不到测试源码 ${TEST_SRC}${NC}"
        exit 1
    fi

    # 1. 编译测试程序为 ELF (不再需要 Makefile，直接调用工具链)
    echo "正在编译测试程序 -> ${TEST_ELF}"
    
    # 检查工具链是否存在
    if ! command -v riscv64-unknown-elf-as &> /dev/null; then
        echo -e "${RED}错误: 未找到 riscv64-unknown-elf-as，请安装 RISC-V 工具链${NC}"
        exit 1
    fi

    # 汇编并链接 (生成 ELF)
    riscv64-unknown-elf-as -o "${BUILD_DIR}/test.o" "${TEST_SRC}"
    if [ $? -ne 0 ]; then echo -e "${RED}汇编失败${NC}"; exit 1; fi
    
    # 链接时指定代码段起始地址，保持与旧测试一致
    riscv64-unknown-elf-ld -Ttext=0x10000 -o "$TEST_ELF" "${BUILD_DIR}/test.o"
    if [ $? -ne 0 ]; then echo -e "${RED}链接失败${NC}"; exit 1; fi

    # 2. 运行模拟器
    echo -e "\n${GREEN}========== 运行模拟器 ==========${NC}"
    "$EMULATOR" "$TEST_ELF"
}

# 主逻辑
case "$1" in
    build)
        build
        ;;
    test)
        build  # 测试前自动检查编译
        test
        ;;
    clean)
        clean
        ;;
    *)
        echo "用法: $0 {build|test|clean}"
        echo "  build - 编译 C++ 模拟器"
        echo "  test  - 编译 RISC-V 测试程序并运行模拟器"
        echo "  clean - 清理所有构建产物"
        exit 1
        ;;
esac

