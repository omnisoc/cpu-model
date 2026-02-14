#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <vector>
#include <cstdint>
#include <string>

// ELF64 结构体定义
namespace ELF {
    const uint8_t EI_NIDENT = 16;
    const uint32_t PT_LOAD = 1;

    struct Elf64_Ehdr {
        uint8_t  e_ident[EI_NIDENT];
        uint16_t e_type;
        uint16_t e_machine;
        uint32_t e_version;
        uint64_t e_entry;
        uint64_t e_phoff;
        uint64_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize;
        uint16_t e_phnum;
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
    };

    struct Elf64_Phdr {
        uint32_t p_type;
        uint32_t p_flags;
        uint64_t p_offset;
        uint64_t p_vaddr;
        uint64_t p_paddr;
        uint64_t p_filesz;
        uint64_t p_memsz;
        uint64_t p_align;
    };
}

class ElfLoader {
public:
    // 加载 ELF 文件到内存
    // 返回值：程序的入口地址
    // 参数：filename - 文件名, memory - 目标内存引用
    static uint64_t load(const std::string& filename, std::vector<uint8_t>& memory);
};

#endif

