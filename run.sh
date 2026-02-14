#!/bin/bash
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'
ROOT_DIR=$(cd "$(dirname "$0")";pwd)
BUILD_DIR="${ROOT_DIR}/build"
BIN_DIR="${ROOT_DIR}/bin"
TEST_DIR="${ROOT_DIR}/test"

clean() {
    echo -e "${YELLOW}Cleaning...${NC}"
    rm -rf "$BUILD_DIR" "$BIN_DIR"
    rm -f "${TEST_DIR}"/*.o "${TEST_DIR}"/*.elf
    echo -e "${GREEN}Clean Done.${NC}"
}

build() {
    echo -e "${YELLOW}[1/2] Building Emulator...${NC}"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    if ! cmake .. > /dev/null; then cmake ..; exit 1; fi
    if make; then echo -e "${GREEN}Emulator Built: ${BIN_DIR}/riscv_emu${NC}"
    else echo -e "${RED}Build Failed!${NC}"; exit 1; fi
}

test_run() {
    echo -e "${YELLOW}[2/2] Running Test...${NC}"
    local EMULATOR="${BIN_DIR}/riscv_emu"
    local TEST_SRC="${TEST_DIR}/test.s"
    local TEST_ELF="${BUILD_DIR}/test.elf"
    if [ ! -f "$EMULATOR" ]; then exit 1; fi
    echo "Compiling RISC-V test program..."
    riscv64-unknown-elf-as -o "${BUILD_DIR}/test.o" "${TEST_SRC}"
    riscv64-unknown-elf-ld -Ttext=0x10000 -o "$TEST_ELF" "${BUILD_DIR}/test.o"
    echo -e "\n${GREEN}========== EXECUTING ==========${NC}"
    "$EMULATOR" "$TEST_ELF"
}

case "$1" in
    build) build ;;
    test) build; test_run ;;
    clean) clean ;;
    *) echo "Usage: $0 {build|test|clean}" ;;
esac
