#!/bin/bash
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'
ROOT_DIR=$(cd "$(dirname "$0")";pwd)
BUILD_DIR="${ROOT_DIR}/build"
BIN_DIR="${ROOT_DIR}/bin"
TEST_SCRIPT="${ROOT_DIR}/test/run_test.sh"

clean() {
    echo -e "${YELLOW}Cleaning...${NC}"
    rm -rf "$BUILD_DIR" "$BIN_DIR"
    echo -e "${GREEN}Clean Done.${NC}"
}

build_emu() {
    if [ ! -d "$BUILD_DIR" ]; then mkdir -p "$BUILD_DIR"; fi
    cd "$BUILD_DIR"
    if [ ! -f Makefile ]; then
        if ! cmake .. > /dev/null; then cmake ..; exit 1; fi
    fi
    
    echo -e "${YELLOW}Building Emulator...${NC}"
    if make; then
        echo -e "${GREEN}Emulator Built.${NC}"
        return 0
    else
        echo -e "${RED}Emulator Build Failed!${NC}"
        return 1
    fi
}

case "$1" in
    build) build_emu ;;
    test) 
        build_emu
        shift
        "$TEST_SCRIPT" test "$@"
        ;;
    gdb) 
        build_emu
        shift
        "$TEST_SCRIPT" gdb "$@"
        ;;
    clean) clean ;;
    *) echo "Usage: $0 {build|test [name]|gdb [name]|clean}" ;;
esac
