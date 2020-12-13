
#pragma once

#include "common/common.h"
#include "boot/stage2/bootconfig.h"

#define ELF_MAGIC 0x464C457FU // "\x7FELF" in little endian

/* e_type definitions */
#define ET_NONE 0
#define ET_REL 1  // Relocatable file
#define ET_EXEC 2 // Executable
#define ET_DYN 3  // Shared object file
#define ET_CORE 4 // Core file

#define EM_386 3 // Intel x86

typedef struct
{
  uint32_t e_ident[4];  /* Magic number and other info */
  uint16_t e_type;      /* Object file type */
  uint16_t e_machine;   /* Architecture */
  uint32_t e_version;   /* Object file version */
  uint32_t e_entry;     /* Entry point virtual address */
  uint32_t e_phoff;     /* Program header table file offset */
  uint32_t e_shoff;     /* Section header table file offset */
  uint32_t e_flags;     /* Processor-specific flags */
  uint16_t e_ehsize;    /* ELF header size in bytes */
  uint16_t e_phentsize; /* Program header table entry size */
  uint16_t e_phnum;     /* Program header table entry count */
  uint16_t e_shentsize; /* Section header table entry size */
  uint16_t e_shnum;     /* Section header table entry count */
  uint16_t e_shstrndx;  /* Section header string table index */
} elf32_ehdr;

#define SHT_NULL 0     /* inactive */
#define SHT_PROGBITS 1 /* Program defined */
#define SHT_SYMTAB 2   /* Symbol table */
#define SHT_STRTAB 3   /* String table */
#define SHT_RELA 4     /* Relocation entries Elf32_Rela */
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8 /* .bss */
#define SHT_REL 9    /* Relocation entries Elf32_Rel */
#define SHT_SHLIB 10
#define SHT_DYNSYM 11

#define SHF_WRITE 1 /* Writable data */
#define SHF_ALLOC 2 /* Allocated memory */
#define SHF_EXEC 4  /* Executable instr */

typedef struct
{
  uint32_t sh_name; /* Index into section header str table */
  uint32_t sh_type; /* Section header type */
  uint32_t sh_flags;
  uint32_t sh_addr;   /* Address the section should appear at */
  uint32_t sh_offset; /* Offset from first byte in file */
  uint32_t sh_size;
  uint32_t sh_link;
  uint32_t sh_info;
  uint32_t sh_addralign; /* Address alignment constraints */
  uint32_t sh_entsize;
} elf32_shdr;

typedef struct
{
  uint32_t p_type;
  uint32_t p_offset; /* offset from beginning of file */
  uint32_t p_vaddr;
  uint32_t p_paddr;
  uint32_t p_filesz; /* bytes of segment in file */
  uint32_t p_memsz;  /* bytes of segment in memory */
  uint32_t p_flags;
  uint32_t p_align;
} elf32_phdr;

// table index 0 is reserved
typedef struct
{
  uint16_t st_name;
  uint32_t st_value;
  uint16_t st_size;
  uint8_t st_info;
  uint8_t st_other;
  uint16_t st_shndx;
} elf32_sym;

#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4

void elf_load(mmap *mem_map, bootconfig *boot_cfg);
void elf_objdump(void *data);
