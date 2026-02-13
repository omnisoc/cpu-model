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
    
    # 清理 CMake 构建目录
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        echo "已删除: ${BUILD_DIR}"
    fi
    
    # 清理可执行文件目录
    if [ -d "$BIN_DIR" ]; then
        rm -rf "$BIN_DIR"
        echo "已删除: ${BIN_DIR}"
    fi
    
    # 清理 test 目录下的中间文件 (如果有)
    if [ -f "${TEST_DIR}/Makefile" ]; then
        cd "$TEST_DIR" && make clean > /dev/null 2>&1
        cd "$ROOT_DIR"
    fi
    
    echo -e "${GREEN}清理完成！${NC}"
}

# 编译函数
build() {
    echo -e "${YELLOW}[1/2] 正在编译模拟器...${NC}"
    
    # 创建构建目录
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir -p "$BUILD_DIR"
    fi
    
    # 进入构建目录执行 cmake 和 make
    cd "$BUILD_DIR" || exit 1
    
    # cmake 输出重定向到 /dev/null，保持界面整洁，如有错误则显示
    if ! cmake .. > /dev/null; then
        echo -e "${RED}CMake 配置失败！${NC}"
        cmake .. # 再次运行以显示错误信息
        exit 1
    fi
    
    # 关键修改：直接运行 make，根据返回值判断成功与否
    # 警告信息会正常显示在屏幕上，但不会导致脚本退出
    if make; then
        # 再次确认可执行文件是否存在
        if [ -f "${BIN_DIR}/riscv_emu" ]; then
            echo -e "${GREEN}编译成功！可执行文件位于: ${BIN_DIR}/riscv_emu${NC}"
        else
            echo -e "${RED}编译过程未报错，但未生成可执行文件！${NC}"
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
    local TEST_BIN="${BUILD_DIR}/test.bin" # 中间文件放在 build 目录

    # 1. 检查模拟器是否存在
    if [ ! -f "$EMULATOR" ]; then
        echo -e "${RED}错误: 找不到模拟器，请先运行 './run.sh build'${NC}"
        exit 1
    fi

    # 2. 检查测试目录是否存在 Makefile
    if [ ! -f "${TEST_DIR}/Makefile" ]; then
        echo -e "${RED}错误: test/ 目录下缺少 Makefile${NC}"
        exit 1
    fi

    # 3. 编译测试程序 (进入 test 目录编译)
    echo "正在编译测试程序..."
    cd "$TEST_DIR" || exit 1
    make clean > /dev/null 2>&1
    if ! make; then
        echo -e "${RED}测试程序编译失败！${NC}"
        exit 1
    fi
    cd "$ROOT_DIR" || exit 1

    # 4. 将生成的 bin 移动到 build 目录 (保持项目整洁)
    if [ -f "${TEST_DIR}/test.bin" ]; then
        mv "${TEST_DIR}/test.bin" "$TEST_BIN"
    else
        echo -e "${RED}错误: test.bin 生成失败${NC}"
        exit 1
    fi

    # 5. 运行模拟器
    echo -e "\n${GREEN}========== 运行模拟器 ==========${NC}"
    "$EMULATOR" "$TEST_BIN"
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

