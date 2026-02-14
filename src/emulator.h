#ifndef EMULATOR_H
#define EMULATOR_H

#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>
// 关键修复：必须包含完整定义，因为 unordered_map 需要知道 TCGBlock 的大小
#include "IR/tcg.h" 

class RiscvEmulator {
public:
    RiscvEmulator();
    bool load_binary(const std::string& filename);
    void run();

private:
    uint64_t regs[32]; 
    uint64_t pc;             
    std::vector<uint8_t> memory;
    bool running;
    
    // 现在编译器知道 IR::TCGBlock 的完整定义了
    std::unordered_map<uint64_t, IR::TCGBlock> tcg_cache;

    uint32_t fetch_instruction();
    uint64_t load_elf(const std::string& filename);
};

#endif
