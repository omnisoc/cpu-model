#include "riscv.h"
#include <iostream>

namespace Arch {
IR::TCGBlock decode(uint32_t insn, [[maybe_unused]] uint64_t pc) {
    IR::TCGBlock block;
    IR::TCGInstruction tcg;
    uint32_t opcode = insn & 0x7F;
    uint32_t rd = (insn >> 7) & 0x1F;
    uint32_t funct3 = (insn >> 12) & 0x7;
    uint32_t rs1 = (insn >> 15) & 0x1F;
    uint32_t funct7 = (insn >> 25) & 0x7F;
    int32_t imm_i = (int32_t)insn >> 20;
    int32_t imm_u = insn & 0xFFFFF000;

    switch (opcode) {
        case 0x37: tcg.op = IR::TCGOp::OP_MOVI; tcg.rd = rd; tcg.rs1 = imm_u; block.insts.push_back(tcg); break;
        case 0x13: if (funct3 == 0x0) { tcg.op = IR::TCGOp::OP_ADDI; tcg.rd = rd; tcg.rs1 = rs1; tcg.rs2 = static_cast<uint32_t>(imm_i); block.insts.push_back(tcg); } break;
        case 0x33: if (funct3 == 0x0) { tcg.op = (funct7 == 0x20) ? IR::TCGOp::OP_SUB : IR::TCGOp::OP_ADD; tcg.rd = rd; tcg.rs1 = rs1; tcg.rs2 = (insn >> 20) & 0x1F; block.insts.push_back(tcg); } break;
        case 0x73: tcg.op = IR::TCGOp::OP_EXIT; block.insts.push_back(tcg); break;
        default: std::cerr << "[Arch] Unknown Opcode: 0x" << std::hex << opcode << std::endl; break;
    }
    return block;
}
}
