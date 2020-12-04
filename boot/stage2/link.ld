OUTPUT_FORMAT(elf32-i386)
ENTRY(stage2_main)
SECTIONS
 {
   . = 0x50000;
   .text : { *(.text) }
   .data : { *(.data) }
   .bss  : { *(.bss)  }
 }
