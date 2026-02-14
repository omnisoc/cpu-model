#!/bin/bash
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

ROOT_DIR=$(cd "$(dirname "$0")/..";pwd)
BUILD_DIR="${ROOT_DIR}/build"
BIN_DIR="${ROOT_DIR}/bin"
TEST_DIR="${ROOT_DIR}/test"
EMULATOR="${BIN_DIR}/cpu_emu"

# ---------------------------------------------------------
# 内部函数: Build
# ---------------------------------------------------------
build_test() {
    local TEST_DIR_NAME=$1
    local TEST_PATH="${TEST_DIR}/${TEST_DIR_NAME}"
    local MAKEFILE="${TEST_PATH}/Makefile"

    local comment=$(grep -m 1 '^# Test:' "$MAKEFILE" 2>/dev/null | sed 's/^# Test: //')
    [ -z "$comment" ] && comment="No description"

    echo -e "${YELLOW}Building Test: ${TEST_DIR_NAME} (${comment})${NC}"

    if [ ! -f "$MAKEFILE" ]; then
        echo -e "${RED}Error: Makefile not found${NC}"
        return 1
    fi

    mkdir -p "$BUILD_DIR"
    
    if ! make -C "$TEST_PATH" BUILD_DIR="${BUILD_DIR}"; then
        echo -e "${RED}Build Failed!${NC}"
        return 1
    fi

    local src_file=$(basename $(ls "${TEST_PATH}"/*.s | head -1) .s)
    local elf_name="${TEST_DIR_NAME}_${src_file}.elf"
    local TEST_ELF="${BUILD_DIR}/${elf_name}"

    if [ ! -f "$TEST_ELF" ]; then
        echo -e "${RED}Error: ELF not found${NC}"
        return 1
    fi

    # 通过全局变量返回结果
    LAST_BUILD_ELF="$TEST_ELF"
}

# ---------------------------------------------------------
# 内部函数: 执行动作
# ---------------------------------------------------------
do_action() {
    local MODE=$1
    local TEST_ELF=$2
    local TEST_NAME=$(basename "$TEST_ELF" .elf)

    if [ ! -f "$EMULATOR" ]; then
        echo -e "${RED}Error: Emulator not found at ${EMULATOR}${NC}"
        return 1
    fi

    case "$MODE" in
        test)
            echo -e "\n${GREEN}========== EXECUTING ${TEST_NAME} ==========${NC}"
            "$EMULATOR" "$TEST_ELF"
            echo -e "${YELLOW}----------------------------------------${NC}"
            ;;
        gdb)
            echo -e "${GREEN}Launching GDB for ${TEST_NAME}...${NC}"
            command gdb --args "$EMULATOR" "$TEST_ELF"
            ;;
    esac
}

# ---------------------------------------------------------
# 内部函数: Clean (仅清理测试产物)
# ---------------------------------------------------------
clean_tests() {
    echo -e "${YELLOW}Cleaning test artifacts...${NC}"
    if [ -d "$BUILD_DIR" ]; then
        rm -f "$BUILD_DIR"/*.elf "$BUILD_DIR"/*.o
        echo -e "${GREEN}Test artifacts cleaned.${NC}"
    else
        echo -e "${GREEN}Build directory does not exist, nothing to clean.${NC}"
    fi
}

# ---------------------------------------------------------
# 菜单逻辑
# ---------------------------------------------------------
show_menu() {
    local MODE=$1
    
    while true; do
        echo -e "${BLUE}========================================${NC}"
        echo -e "${BLUE}       CPU Emulator Test Suite         ${NC}"
        echo -e "${BLUE}========================================${NC}"
        
        declare -a TEST_DIRS
        local idx=1
        
        for dir in "${TEST_DIR}"/*/ ; do
            if [ -f "${dir}Makefile" ]; then
                local dir_name=$(basename "$dir")
                local comment=$(grep -m 1 '^# Test:' "${dir}Makefile" | sed 's/^# Test: //')
                [ -z "$comment" ] && comment="No description"
                
                TEST_DIRS[$idx]=$dir_name
                printf "${GREEN}%2d${NC}) %-15s - %s\n" "$idx" "$dir_name" "$comment"
                ((idx++))
            fi
        done

        # --- 修复点：此处必须是 ] ---
        if [ ${#TEST_DIRS[@]} -eq 0 ]; then
            echo -e "${RED}No tests found.${NC}"
            exit 1
        fi

        echo -e "${YELLOW} c${NC}) Clean All Tests"
        echo -e "${RED} q${NC}) Exit"
        echo -e "${BLUE}========================================${NC}"
        
        read -p "Select> " choice

        case $choice in
            c|C)
                clean_tests
                ;;
            q|Q) 
                echo "Exiting..."
                exit 0 
                ;;
            *)
                if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le ${#TEST_DIRS[@]} ]; then
                    local selected_name="${TEST_DIRS[$choice]}"
                    
                    # 1. 编译
                    build_test "$selected_name"
                    
                    # 2. 根据模式执行
                    if [ -n "$LAST_BUILD_ELF" ] && [ -f "$LAST_BUILD_ELF" ]; then
                        do_action "$MODE" "$LAST_BUILD_ELF"
                        exit 0
                    fi
                else
                    echo -e "${RED}Invalid choice.${NC}"
                fi
                ;;
        esac
    done
}

# ---------------------------------------------------------
# 入口
# ---------------------------------------------------------
mkdir -p "$BUILD_DIR"

CMD=$1
TARGET=$2

case "$CMD" in
    test)
        if [ -n "$TARGET" ]; then
            build_test "$TARGET"
            [ -n "$LAST_BUILD_ELF" ] && do_action "test" "$LAST_BUILD_ELF"
        else
            show_menu "test"
        fi
        ;;
    gdb)
        if [ -n "$TARGET" ]; then
            build_test "$TARGET"
            [ -n "$LAST_BUILD_ELF" ] && do_action "gdb" "$LAST_BUILD_ELF"
        else
            show_menu "gdb"
        fi
        ;;
    *)
        show_menu "test"
        ;;
esac
