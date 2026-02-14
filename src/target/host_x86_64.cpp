#include "host_x86_64.h"
#include <vector>
#include <sys/mman.h>
#include <iostream>
#include <string.h>

namespace Target {
class CodeGen {
    std::vector<uint8_t> code;
public:
    void emit_byte(uint8_t b) { code.push_back(b); }
    void emit32(uint32_t val) { code.insert(code.end(), (uint8_t*)&val, (uint8_t*)&val + 4); }
    void emit64(uint64_t val) { code.insert(code.end(), (uint8_t*)&val, (uint8_t*)&val + 8); }
    void emit_mov_rax_imm64(uint64_t imm) { emit_byte(0x48); emit_byte(0xB8); emit64(imm); }
    void emit_mov_to_reg_file(uint32_t reg_idx) {
        if (reg_idx == 0) return; 
        emit_byte(0x48); emit_byte(0x89); emit_byte(0x47); emit_byte(reg_idx * 8);
    }
    void emit_mov_from_reg_file(uint32_t reg_idx) {
        emit_byte(0x48); emit_byte(0x8B); emit_byte(0x47); emit_byte(reg_idx * 8);
    }
    void emit_add_from_reg_file(uint32_t reg_idx) {
        emit_byte(0x48); emit_byte(0x03); emit_byte(0x47); emit_byte(reg_idx * 8);
    }
    void emit_sub_from_reg_file(uint32_t reg_idx) {
        emit_byte(0x48); emit_byte(0x2B); emit_byte(0x47); emit_byte(reg_idx * 8);
    }
    void emit_ret() { emit_byte(0xC3); }
    uint8_t* data() { return code.data(); }
    size_t size() { return code.size(); }
};

void execute(const IR::TCGBlock& block, uint64_t* regs, bool& running) {
    CodeGen gen;
    for (const auto& insn : block.insts) {
        switch (insn.op) {
            case IR::TCGOp::OP_MOVI:
                gen.emit_mov_rax_imm64(insn.rs1); gen.emit_mov_to_reg_file(insn.rd); break;
            case IR::TCGOp::OP_ADDI:
                gen.emit_mov_from_reg_file(insn.rs1);
                gen.emit_byte(0x48); gen.emit_byte(0x05); gen.emit32(static_cast<int32_t>(insn.rs2));
                gen.emit_mov_to_reg_file(insn.rd); break;
            case IR::TCGOp::OP_ADD:
                gen.emit_mov_from_reg_file(insn.rs1); gen.emit_add_from_reg_file(insn.rs2);
                gen.emit_mov_to_reg_file(insn.rd); break;
            case IR::TCGOp::OP_SUB:
                gen.emit_mov_from_reg_file(insn.rs1); gen.emit_sub_from_reg_file(insn.rs2);
                gen.emit_mov_to_reg_file(insn.rd); break;
            case IR::TCGOp::OP_EXIT:
                gen.emit_byte(0xC6); gen.emit_byte(0x06); gen.emit_byte(0x00); break;
            default: break;
        }
    }
    gen.emit_ret();
    void* exec_mem = mmap(NULL, gen.size(), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (exec_mem == MAP_FAILED) return;
    memcpy(exec_mem, gen.data(), gen.size());
    typedef void (*jit_func_t)(uint64_t*, bool*);
    jit_func_t func = (jit_func_t)exec_mem;
    func(regs, &running);
    munmap(exec_mem, gen.size());
}
}
