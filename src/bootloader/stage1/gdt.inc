%ifndef GDT_INC
%define GDT_INC

%define GDT_MAX_LIMIT 0xffff ; I just use the max limit lel
%define GDT_PRESENT 0b10000000 ; Is present
%define GDT_RING3 0b01100000 ; Privilege level 3
%define GDT_DESCRIPTOR 0b00010000 ; Descriptor type, set for code/data
%define GDT_EXECUTABLE 0b00001000 ; Can be executed
%define GDT_READWRITE 0b00000010 ; Read/write access for code/data
%define GDT_ACCESSED 0b00000001 ; Whether segment is accessed
%define GDT_GRANULARITY (0x80 | 0) ; Page granularity (4KiB)
%define GDT_SIZE (0x40 | 0) ; Use 32 bit selectors

install_gdt:
  pusha ; save registers
  lgdt 	[gdt_desc] ; load GDT into GDTR
  popa ; restore registers
  ret

; GLOBAL DESCRIPTOR TABLE
gdt: ; Address for the GDT

gdt_null:
  dd 0
  dd 0

gdt_kernel_code:
  dw GDT_MAX_LIMIT ; Limit
  dw 0 ; First base
  db 0 ; Second base
  db (GDT_PRESENT | GDT_DESCRIPTOR | GDT_EXECUTABLE | GDT_READWRITE) ; Configuration
  db (GDT_GRANULARITY | GDT_SIZE) ; Flags
  db 0 ; Third base

gdt_kernel_data:
  dw GDT_MAX_LIMIT ; Limit
  dw 0 ; First base
  db 0 ; Second base
  db (GDT_PRESENT | GDT_DESCRIPTOR | GDT_READWRITE) ; Configuration
  db (GDT_GRANULARITY | GDT_SIZE) ; Flags
  db 0 ; Third base

gdt_end: ; Used to calculate the size of the GDT

gdt_desc: ; The GDT descriptor
  dw gdt_end - gdt - 1 ; Limit (size)
  dd gdt ; Address of the GDT

%endif
