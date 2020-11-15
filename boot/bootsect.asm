[org 0x7c00]
  KERNEL_OFFSET equ 0x1000 ; The same one we used when linking the kernel
  KERNEL_SECTORS_COUNT equ 50 ; Number of sectiors to load when loading kernel

  jmp 0x00:boot ; Sets cs to 0.

boot:
  xor ax, ax ; "XORing" the same register/segment/pointer sets that register/segment/pointer to 0, to we set ax as 0.
  mov es, ax ; we set ES to 0, because AX has the same value
  mov ds, ax ; set ds to 0
  mov ss, ax ; set ss to 0

  mov bp, 0x7c00 ; Right before the bootloader.
  mov sp, bp ; we set sp to 0x7C00

  mov bx, MSG_REAL_MODE
  call print
  call print_nl

  mov [BOOT_DRIVE], dl ; Remember that the BIOS sets us the boot drive in 'dl' on boot
  call load_kernel ; read the kernel from disk

  call switch_to_pm ; disable interrupts, load GDT,  etc. Finally jumps to 'BEGIN_PM'
  jmp $ ; Never executed

%include "boot/print.asm"
%include "boot/print_hex.asm"
%include "boot/disk.asm"
%include "boot/gdt.asm"
%include "boot/32bit_print.asm"
%include "boot/switch_pm.asm"

[bits 16]
load_kernel:
  mov bx, MSG_LOAD_KERNEL
  call print
  call print_nl

  mov bx, KERNEL_OFFSET ; Read from disk and store in 0x1000
  mov dh, KERNEL_SECTORS_COUNT
  mov dl, [BOOT_DRIVE]
  call disk_load
  ret

[bits 32]
BEGIN_PM:
  mov ebx, MSG_PROT_MODE
  call print_string_pm
  call KERNEL_OFFSET ; Give control to the kernel
  jmp $ ; Stay here when the kernel returns control to us (if ever)


BOOT_DRIVE db 0 ; It is a good idea to store it in memory because 'dl' may get overwritten
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
MSG_PROT_MODE db "Landed in 32-bit Protected Mode", 0
MSG_LOAD_KERNEL db "Loading kernel into memory", 0

; padding
times 510 - ($-$$) db 0
dw 0xaa55
