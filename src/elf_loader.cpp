#include "elf_loader.h"
#include <fstream>
#include <iostream>
#include <cstring>

uint64_t ElfLoader::load(const std::string& filename, std::vector<uint8_t>& memory) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[Loader] 无法打开文件: " << filename << std::endl;
        return 0;
    }

    // 1. 读取 ELF Header
    ELF::Elf64_Ehdr ehdr;
    file.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr));

    // 2. 校验魔数
    if (ehdr.e_ident[0] != 0x7F || ehdr.e_ident[1] != 'E' || 
        ehdr.e_ident[2] != 'L' || ehdr.e_ident[3] != 'F') {
        std::cerr << "[Loader] 错误: 不是有效的 ELF 文件" << std::endl;
        return 0;
    }

    // 简单检查是否为 RISC-V 架构 (0xF3)
    if (ehdr.e_machine != 0xF3) {
        std::cerr << "[Loader] 警告: 架构非 RISC-V (Machine: 0x" 
                  << std::hex << ehdr.e_machine << ")" << std::endl;
    }

    std::cout << "[Loader] ELF 入口点: 0x" << std::hex << ehdr.e_entry << std::endl;

    // 3. 遍历并加载段
    for (int i = 0; i < ehdr.e_phnum; i++) {
        file.seekg(ehdr.e_phoff + i * ehdr.e_phentsize, std::ios::beg);
        
        ELF::Elf64_Phdr phdr;
        file.read(reinterpret_cast<char*>(&phdr), sizeof(phdr));

        if (phdr.p_type == ELF::PT_LOAD) {
            // 边界检查
            if (phdr.p_vaddr + phdr.p_memsz > memory.size()) {
                std::cerr << "[Loader] 错误: 段溢出内存 (Vaddr: 0x" << std::hex << phdr.p_vaddr 
                          << ", Size: 0x" << phdr.p_memsz << ")" << std::endl;
                continue; // 或者 return 0;
            }

            // 复制文件数据到内存
            if (phdr.p_filesz > 0) {
                file.seekg(phdr.p_offset, std::ios::beg);
                file.read(reinterpret_cast<char*>(&memory[phdr.p_vaddr]), phdr.p_filesz);
            }

            // .bss 段处理: 剩余空间清零
            if (phdr.p_memsz > phdr.p_filesz) {
                memset(&memory[phdr.p_vaddr + phdr.p_filesz], 0, phdr.p_memsz - phdr.p_filesz);
            }

            std::cout << "[Loader] 加载段 " << i << ": [0x" << std::hex << phdr.p_vaddr 
                      << " - 0x" << (phdr.p_vaddr + phdr.p_memsz) << "]" << std::endl;
        }
    }

    return ehdr.e_entry; // 返回入口地址
}

