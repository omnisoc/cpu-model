#include "emulator.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <riscv_binary>" << std::endl;
        std::cerr << "请提供一个 RISC-V 可执行程序 (flat binary 格式)" << std::endl;
        return 1;
    }

    RiscvEmulator emu;
    
    if (!emu.load_binary(argv[1])) {
        return 1;
    }
    
    emu.run();
    
    return 0;
}

