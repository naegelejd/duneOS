#ifndef DUNE_ELF_H
#define DUNE_ELF_H

#include "dune.h"

typedef uintptr_t elf32_addr;
typedef uint16_t elf32_half;
typedef uint32_t elf32_off;
typedef int32_t elf32_sword;
typedef uint32_t elf32_word;

enum { EI_IDENT = 16 };

enum e_type {
    ET_NONE = 0, ET_REAL, ET_EXEC, ET_DYN, ET_CORE,
    ET_LOPROC = 0xFF00, ET_HIPROC = 0xFFFF
};

enum e_machine {
    EM_NONE = 0, EM_M32, EM_SPARC, EM_386,
    EM_68K, EM_88K, EM_860, EM_MIPS
};

enum e_version { EV_NONE = 0, EV_CURRENT = 1 };

struct elf32_hdr {
    uint8_t eident[EI_IDENT];   /* ELF identification */
    elf32_half e_type;          /* obj file type */
    elf32_half e_machine;       /* obj machine type */
    elf32_word e_version;       /* ELF version */
    elf32_addr e_entry;         /* virtual address of entry point */
    elf32_off e_phoff;          /* program header table's file offset */
    elf32_off e_shoff;          /* section header table's file offset */
    elf32_word e_flags;         /* processor-specific flags */
    elf32_half e_ehsize;        /* ELF header's size in bytes */
    elf32_half e_phentsize;     /* size in bytes of one entry in program header */
    elf32_half e_phnum;         /* number of entries in program header */
    elf32_half e_shentsize;     /* section header size in bytes */
    elf32_half e_shnum;         /* number of entries in section header */
    elf32_half e_shstrndx;      /* sh table index of entry for section name string table */
};

#endif /* DUNE_ELF_H */
