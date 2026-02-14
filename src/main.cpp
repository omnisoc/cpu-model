#include "emulator.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <riscv_elf>" << std::endl; return 1; }
    RiscvEmulator emu;
    if (!emu.load_binary(argv[1])) return 1;
    emu.run();
    return 0;
}
