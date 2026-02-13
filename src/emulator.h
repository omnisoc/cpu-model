#ifndef EMULATOR_H
#define EMULATOR_H

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <functional>
#include <stdexcept>

const int NUM_REGS = 32;

enum class TCGOp {
    OP_MOVI,    // 立即数赋值 rd = imm
    OP_ADD,     // 寄存器加法 rd = rs1 + rs2
    OP_ADDI,    // 立即数加法 rd = rs1 + imm  <-- 新增
    OP_SUB,     // 减法
    OP_LD,      
    OP_ST,      
    OP_EXIT     
};

struct TCGInstruction {
    TCGOp op;
    uint32_t rd;    
    uint32_t rs1;   
    uint32_t rs2;   // 寄存器索引 或 立即数
};

class RiscvEmulator {
public:
    RiscvEmulator();
    bool load_binary(const std::string& filename);
    void run();

private:
    uint64_t regs[NUM_REGS]; 
    uint64_t pc;             
    std::vector<uint8_t> memory;
    std::unordered_map<uint64_t, std::vector<TCGInstruction>> tcg_cache;
    
    void tcg_translate(uint64_t vaddr);
    void tcg_execute(const std::vector<TCGInstruction>& block);
    uint32_t fetch_instruction();
    bool running;
};

#endif

