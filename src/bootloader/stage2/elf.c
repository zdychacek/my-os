#include "bootloader/stage2/lib.h"
#include "bootloader/stage2/ext2.h"
#include "bootloader/stage2/elf.h"
#include "bootloader/stage2/vga.h"
#include "lib/string.h"
#include "lib/memory.h"

extern uint32_t HEAP_START;

void elf_objdump(void *data)
{
  elf32_ehdr *ehdr = (elf32_ehdr *)data;

  assert(ehdr->e_ident[0] == ELF_MAGIC);
  assert(ehdr->e_machine == EM_386);
  assert(ehdr->e_type == ET_EXEC);

  printx("ELF ident ", ehdr->e_ident[0]);
  printx("Entry ", ehdr->e_entry);

  // Parse the program headers
  elf32_phdr *phdr = (elf32_phdr *)((uint32_t)data + ehdr->e_phoff);
  elf32_phdr *last_phdr = (elf32_phdr *)((uint32_t)phdr + (ehdr->e_phentsize * ehdr->e_phnum));
  vga_pretty("Offset   \tVirt Addr\tPhys Addr\tFile Sz\tMem sz \tAlign  \n", VGA_LIGHTMAGENTA);

  char buf[32];

  while (phdr < last_phdr)
  {
    vga_puts("   ");
    vga_puts(itoa(phdr->p_offset, buf, 16));
    vga_puts("\t");
    vga_puts(itoa(phdr->p_vaddr, buf, 16));
    vga_putc('\t');
    vga_puts(itoa(phdr->p_paddr, buf, 16));
    vga_putc('\t');
    vga_puts(itoa(phdr->p_filesz, buf, 10));
    vga_putc('\t');
    vga_puts(itoa(phdr->p_memsz, buf, 16));
    vga_putc('\t');
    vga_putc('\n');

    phdr++;
  }

  // Parse the section headers
  elf32_shdr *shdr = (elf32_shdr *)((uint32_t)data + ehdr->e_shoff);
  elf32_shdr *sh_str = (elf32_shdr *)((uint32_t)shdr + (ehdr->e_shentsize * ehdr->e_shstrndx));
  elf32_shdr *last_shdr = (elf32_shdr *)((uint32_t)shdr + (ehdr->e_shentsize * ehdr->e_shnum));

  char *string_table = (char *)((uint32_t)data + sh_str->sh_offset);

  shdr++; // Skip null entry
  int q = 0;

  vga_pretty("Sections:\nIdx   Name\tSize\t\tAddress \tOffset\tAlign\n", VGA_LIGHTMAGENTA);

  while (shdr < last_shdr)
  {
    vga_puts(itoa(++q, buf, 10));
    vga_puts("  ");
    vga_puts(string_table + shdr->sh_name);

    if (strlen(string_table + shdr->sh_name) < 6)
    {
      vga_puts("\t");
    }
    vga_putc('\t');
    vga_puts(itoa(shdr->sh_size, buf, 16));
    vga_putc('\t');
    vga_puts(itoa(shdr->sh_addr, buf, 16));
    vga_putc('\t');
    vga_puts(itoa(shdr->sh_offset, buf, 10));
    vga_putc('\t');
    vga_puts(itoa(shdr->sh_addralign, buf, 16));
    vga_putc('\n');

    shdr++;
  }
}

void elf_load(multiboot_info *bootinfo, bootconfig *boot_cfg)
{
  int kernel_inode = ext2_find_child(boot_cfg->file, 2);

  if (kernel_inode == -1)
  {
    vga_pretty("Cannot find kernel file.", VGA_RED);

    return;
  }

  uint32_t *data = ext2_read_file(ext2_inode(1, kernel_inode));
  elf32_ehdr *ehdr = (elf32_ehdr *)data;

  assert(ehdr->e_ident[0] == ELF_MAGIC);

  elf_objdump(data);
  printx("data at: ", (uint32_t)data);
  printx("heap at: ", (uint32_t)malloc(0));
  free(data);

  elf32_phdr *phdr = (elf32_phdr *)((uint32_t)data + ehdr->e_phoff);
  elf32_phdr *last_phdr = (elf32_phdr *)((uint32_t)phdr + (ehdr->e_phentsize * ehdr->e_phnum));

  uint32_t off = (phdr->p_vaddr - phdr->p_paddr);

  while (phdr < last_phdr)
  {
    printx("header: ", phdr->p_paddr);
    memcpy((void *)phdr->p_paddr, (void *)((uint32_t)data + phdr->p_offset), phdr->p_filesz);
    phdr++;
  }

  void (*entry)(unsigned long, multiboot_info *) = (void *)(ehdr->e_entry - off);
  printx("entry: ", (uint32_t)entry);

  // CLEAR OUT THE ENTIRE HEAP
  uint32_t END_OF_HEAP = (uint32_t)malloc(0);
  memset((void *)HEAP_START, 0, (END_OF_HEAP - HEAP_START));

  asm volatile("cli");
  entry(0x2BADB002, bootinfo);
}
