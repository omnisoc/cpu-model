#include "emulator.h"
#include "arch/riscv.h"
#include "target/host_x86_64.h"
#include "IR/tcg.h"
#include <fstream>
#include <iostream>
#include <cstring>

namespace ELF {
    const int EI_NIDENT = 16;
    const uint32_t PT_LOAD = 1;
    struct Elf64_Ehdr { uint8_t e_ident[EI_NIDENT]; uint16_t e_type; uint16_t e_machine; uint32_t e_version; uint64_t e_entry; uint64_t e_phoff; uint64_t e_shoff; uint32_t e_flags; uint16_t e_ehsize; uint16_t e_phentsize; uint16_t e_phnum; uint16_t e_shentsize; uint16_t e_shnum; uint16_t e_shstrndx; };
    struct Elf64_Phdr { uint32_t p_type; uint32_t p_flags; uint64_t p_offset; uint64_t p_vaddr; uint64_t p_paddr; uint64_t p_filesz; uint64_t p_memsz; uint64_t p_align; };
}

RiscvEmulator::RiscvEmulator() {
    memset(regs, 0, sizeof(regs)); pc = 0; running = true;
    memory.resize(128 * 1024 * 1024, 0);
    regs[2] = 0x8000000 - 0x10;
}

uint64_t RiscvEmulator::load_elf(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return 0;
    ELF::Elf64_Ehdr ehdr;
    file.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr));
    if (ehdr.e_ident[0] != 0x7F || ehdr.e_ident[1] != 'E') return 0;
    std::cout << "[Loader] ELF Entry: 0x" << std::hex << ehdr.e_entry << std::endl;
    for (int i = 0; i < ehdr.e_phnum; i++) {
        file.seekg(ehdr.e_phoff + i * ehdr.e_phentsize);
        ELF::Elf64_Phdr phdr;
        file.read(reinterpret_cast<char*>(&phdr), sizeof(phdr));
        if (phdr.p_type == ELF::PT_LOAD) {
            if (phdr.p_vaddr + phdr.p_memsz > memory.size()) continue;
            file.seekg(phdr.p_offset);
            file.read(reinterpret_cast<char*>(&memory[phdr.p_vaddr]), phdr.p_filesz);
            if (phdr.p_memsz > phdr.p_filesz)
                memset(&memory[phdr.p_vaddr + phdr.p_filesz], 0, phdr.p_memsz - phdr.p_filesz);
        }
    }
    return ehdr.e_entry;
}

bool RiscvEmulator::load_binary(const std::string& filename) {
    uint64_t entry = load_elf(filename);
    if (entry) { pc = entry; return true; }
    return false;
}

uint32_t RiscvEmulator::fetch_instruction() {
    if (pc >= memory.size()) throw std::runtime_error("PC Out of Bounds");
    return *reinterpret_cast<uint32_t*>(&memory[pc]);
}

void RiscvEmulator::run() {
    std::cout << "Starting Execution..." << std::endl;
    while (running) {
        uint64_t current_pc = pc;
        if (tcg_cache.find(current_pc) == tcg_cache.end()) {
            uint32_t insn = fetch_instruction();
            IR::TCGBlock block = Arch::decode(insn, current_pc);
            tcg_cache[current_pc] = block;
            pc += 4;
        } else { pc += 4; }
        if (tcg_cache.count(current_pc)) {
            Target::execute(tcg_cache[current_pc], regs, running);
        }
        if (pc > 0x10000 + 10000) break; 
    }
    std::cout << "\nFinished. Registers:" << std::endl;
    for (int i = 0; i < 32; i++) {
        std::cout << "x" << i << ": 0x" << std::hex << regs[i] << "  ";
        if ((i + 1) % 4 == 0) std::cout << std::endl;
    }
}
