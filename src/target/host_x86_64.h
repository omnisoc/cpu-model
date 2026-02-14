#ifndef TARGET_HOST_X86_64_H
#define TARGET_HOST_X86_64_H
#include "IR/tcg.h"
#include <cstdint>
namespace Target {
    void execute(const IR::TCGBlock& block, uint64_t* regs, bool& running);
}
#endif
