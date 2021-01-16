OUTPUT_FORMAT(elf32-i386)
ENTRY(loader)
SECTIONS
{
  . = 0xC0100000;
  kernel_virt_start = .;
  kernel_start = . - 0xC0000000;

  .text BLOCK(4K) : AT(ADDR(.text) - 0xC0000000)
  {
    *(.text)
    *(.rodata)
  }
  .data BLOCK(4K) : AT(ADDR(.data) - 0xC0000000)
  {
    *(.data)
  }
  .bss BLOCK(4K) : AT(ADDR(.bss) - 0xC0000000)
  {
    *(COMMON)
    *(.bss)
  }
  kernel_virt_end = .;
  kernel_end = . - 0xC0000000;
}
