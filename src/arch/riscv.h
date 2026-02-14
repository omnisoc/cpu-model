#ifndef ARCH_RISCV_H
#define ARCH_RISCV_H
#include "IR/tcg.h"
#include <cstdint>
namespace Arch {
    IR::TCGBlock decode(uint32_t insn, uint64_t pc);
}
#endif
