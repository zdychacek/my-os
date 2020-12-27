OUTPUT_FORMAT(elf32-i386)
ENTRY(kmain)
SECTIONS
 {
   . = 1M;
   kernel_start = .;

  .text BLOCK(4K) : ALIGN(4K)
  {
    *(.text)
  }
  .rodata BLOCK(4K) : ALIGN(4K)
  {
	  *(.rodata)
  }
  .data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
  .bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
  kernel_end = .;
 }
