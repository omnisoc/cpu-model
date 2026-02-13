#include "emulator.h"
#include <fstream>
#include <iostream>
#include <cstring>

RiscvEmulator::RiscvEmulator() {
    memset(regs, 0, sizeof(regs));
    pc = 0;
    running = true;
    memory.resize(128 * 1024 * 1024, 0);
    regs[2] = 0x8000000; // x2 (sp)
}

bool RiscvEmulator::load_binary(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    uint64_t load_addr = 0x10000;
    if (static_cast<size_t>(size) > memory.size() - load_addr) {
        std::cerr << "程序太大" << std::endl;
        return false;
    }
    file.read(reinterpret_cast<char*>(&memory[load_addr]), size);
    pc = load_addr;
    regs[3] = load_addr; 
    std::cout << "程序已加载到 0x" << std::hex << load_addr 
              << ", 大小: " << std::dec << size << " bytes" << std::endl;
    return true;
}

uint32_t RiscvEmulator::fetch_instruction() {
    if (pc >= memory.size()) throw std::runtime_error("PC 超出范围");
    return *reinterpret_cast<uint32_t*>(&memory[pc]);
}

void RiscvEmulator::tcg_translate(uint64_t vaddr) {
    uint32_t insn = fetch_instruction();
    std::vector<TCGInstruction> block;
    
    uint32_t opcode = insn & 0x7F;
    uint32_t rd = (insn >> 7) & 0x1F;
    uint32_t funct3 = (insn >> 12) & 0x7;
    uint32_t rs1 = (insn >> 15) & 0x1F;
    uint32_t funct7 = (insn >> 25) & 0x7F;
    
    // 立即数提取
    int32_t imm_i = (int32_t)insn >> 20;
    int32_t imm_u = insn & 0xFFFFF000;
    
    TCGInstruction tcg;
    
    switch (opcode) {
        case 0x37: // LUI
            tcg.op = TCGOp::OP_MOVI;
            tcg.rd = rd;
            tcg.rs1 = imm_u; 
            block.push_back(tcg);
            pc += 4;
            break;
            
        case 0x13: // OP-IMM (ADDI, etc.)
            if (funct3 == 0x0) { // ADDI
                tcg.op = TCGOp::OP_ADDI;
                tcg.rd = rd;
                tcg.rs1 = rs1;
                tcg.rs2 = static_cast<uint32_t>(imm_i); // 保存立即数
                block.push_back(tcg);
            }
            pc += 4;
            break;
            
        case 0x33: // OP (ADD, SUB)
            if (funct3 == 0x0) {
                tcg.op = (funct7 == 0x20) ? TCGOp::OP_SUB : TCGOp::OP_ADD;
                tcg.rd = rd;
                tcg.rs1 = rs1;
                tcg.rs2 = (insn >> 20) & 0x1F; // rs2 索引
                block.push_back(tcg);
            }
            pc += 4;
            break;

        case 0x6F: // JAL
            tcg.op = TCGOp::OP_MOVI;
            tcg.rd = rd;
            tcg.rs1 = pc + 4;
            block.push_back(tcg);
            pc += 4;
            break;

        case 0x73: // ECALL
            std::cout << "[TCG] 检测到 ECALL，准备退出" << std::endl;
            tcg.op = TCGOp::OP_EXIT;
            block.push_back(tcg);
            pc += 4;
            break;
            
        default:
            std::cerr << "未实现指令 Opcode: 0x" << std::hex << opcode 
                      << " at PC: 0x" << pc << std::endl;
            pc += 4;
            break;
    }
    tcg_cache[vaddr] = block;
}

void RiscvEmulator::tcg_execute(const std::vector<TCGInstruction>& block) {
    for (const auto& insn : block) {
        switch (insn.op) {
            case TCGOp::OP_MOVI:
                if (insn.rd != 0) regs[insn.rd] = insn.rs1;
                break;
            case TCGOp::OP_ADD: // 寄存器加法
                if (insn.rd != 0) regs[insn.rd] = regs[insn.rs1] + regs[insn.rs2];
                break;
            case TCGOp::OP_ADDI: // 立即数加法
                if (insn.rd != 0) {
                    // 注意符号扩展：rs2 存的是 uint32_t，需要转回 int32_t 再转 int64_t
                    int64_t imm = static_cast<int32_t>(insn.rs2);
                    regs[insn.rd] = regs[insn.rs1] + imm;
                }
                break;
            case TCGOp::OP_SUB:
                if (insn.rd != 0) regs[insn.rd] = regs[insn.rs1] - regs[insn.rs2];
                break;
            case TCGOp::OP_EXIT:
                running = false;
                return;
            default:
                break;
        }
    }
}

void RiscvEmulator::run() {
    std::cout << "开始执行..." << std::endl;
    while (running) {
        uint64_t current_pc = pc;
        if (tcg_cache.find(current_pc) == tcg_cache.end()) {
            tcg_translate(current_pc);
        }
        if (tcg_cache.count(current_pc)) {
            tcg_execute(tcg_cache[current_pc]);
        }
        if (pc > 0x10000 + 10000) break;
    }
    
    std::cout << "\n执行结束。寄存器状态:" << std::endl;
    for (int i = 0; i < 32; i++) {
        std::cout << "x" << i << ": 0x" << std::hex << regs[i] << "  ";
        if ((i + 1) % 4 == 0) std::cout << std::endl;
    }
}

