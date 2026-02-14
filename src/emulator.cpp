#include "emulator.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>

// --- ELF64 结构定义 (内置于 cpp 中) ---
namespace ELF {
    const int EI_NIDENT = 16;
    const uint32_t PT_LOAD = 1;

    struct Elf64_Ehdr {
        uint8_t  e_ident[EI_NIDENT];
        uint16_t e_type;
        uint16_t e_machine;  // 0xF3 = RISC-V
        uint32_t e_version;
        uint64_t e_entry;    // 入口点
        uint64_t e_phoff;    // Program Header 偏移
        uint64_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize;
        uint16_t e_phnum;    // Program Header 数量
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
    };

    struct Elf64_Phdr {
        uint32_t p_type;     // 段类型
        uint32_t p_flags;
        uint64_t p_offset;   // 文件偏移
        uint64_t p_vaddr;    // 虚拟地址
        uint64_t p_paddr;
        uint64_t p_filesz;   // 文件大小
        uint64_t p_memsz;    // 内存大小
        uint64_t p_align;
    };
}

RiscvEmulator::RiscvEmulator() {
    memset(regs, 0, sizeof(regs));
    pc = 0;
    running = true;
    memory.resize(128 * 1024 * 1024, 0);
    regs[2] = 0x8000000 - 0x10; // 初始化栈指针 到内存末尾
}

bool RiscvEmulator::load_binary(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }

    // 1. 尝试读取 ELF Header
    ELF::Elf64_Ehdr ehdr;
    file.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr));

    // 2. 校验 ELF 魔数
    bool is_elf = (ehdr.e_ident[0] == 0x7F && ehdr.e_ident[1] == 'E' && 
                   ehdr.e_ident[2] == 'L' && ehdr.e_ident[3] == 'F');

    if (is_elf) {
        std::cout << "[Loader] 检测到 ELF 文件..." << std::endl;
        if (ehdr.e_machine != 0xF3) {
            std::cerr << "[Loader] 警告: 架构不匹配 (期望 RISC-V 0xF3, 实际 0x" 
                      << std::hex << ehdr.e_machine << ")" << std::endl;
        }
        
        // 遍历 Program Headers
        for (int i = 0; i < ehdr.e_phnum; i++) {
            file.seekg(ehdr.e_phoff + i * ehdr.e_phentsize, std::ios::beg);
            ELF::Elf64_Phdr phdr;
            file.read(reinterpret_cast<char*>(&phdr), sizeof(phdr));

            if (phdr.p_type == ELF::PT_LOAD) {
                if (phdr.p_vaddr + phdr.p_memsz > memory.size()) {
                    std::cerr << "[Loader] 错误: 内存越界" << std::endl;
                    return false;
                }
                
                // 读取段数据
                file.seekg(phdr.p_offset, std::ios::beg);
                file.read(reinterpret_cast<char*>(&memory[phdr.p_vaddr]), phdr.p_filesz);
                
                // .bss 清零
                if (phdr.p_memsz > phdr.p_filesz) {
                    memset(&memory[phdr.p_vaddr + phdr.p_filesz], 0, phdr.p_memsz - phdr.p_filesz);
                }
                std::cout << "[Loader] 加载段 " << i << " [0x" << std::hex << phdr.p_vaddr 
                          << ", 0x" << (phdr.p_vaddr + phdr.p_memsz) << "]" << std::dec << std::endl;
            }
        }
        pc = ehdr.e_entry;
        std::cout << "[Loader] 入口点设置: 0x" << std::hex << pc << std::dec << std::endl;
        return true;
    }

    // 3. 如果不是 ELF，回退到 Flat Binary 模式
    std::cout << "[Loader] 非 ELF 文件，尝试作为 Flat Binary 加载..." << std::endl;
    std::streamsize size = file.tellg(); // 之前读了一个 header，获取大小不准，需重置
    file.clear();
    file.seekg(0, std::ios::end);
    size = file.tellg();
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

// ... fetch_instruction, tcg_translate, tcg_execute, run 保持不变 ...
// (为了节省篇幅，未列出的部分与原版本完全一致)


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

