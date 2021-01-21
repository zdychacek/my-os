[bits 16]

%include "src/bootloader/lib/constants.inc"

%define MULTIBOOT_FLAGS_MEM 1
%define MULTIBOOT_FLAGS_BOOTDEVICE 0b10
%define MULTIBOOT_FLAGS_MMAP 0b100000
%define MULTIBOOT_FLAGS (MULTIBOOT_FLAGS_MEM | MULTIBOOT_FLAGS_BOOTDEVICE | MULTIBOOT_FLAGS_MMAP)
%define STAGE2_POSITION 0x50000 ; Loaded Stage 2 position
%define STACK_POINTER 0x300000 ; The initial stack pointer for Stage 2 loader
%define GDT_CODE_OFFSET 0x8 ; Offset to GDT code segment
%define GDT_DATA_OFFSET 0x10 ; Offset to GDT data segment

global _start

_start:
  mov [drive], dx

  ; Load ext2 superblock
  mov dword [packet + DAP.lba_lo], 2
  mov dword [packet + DAP.dest_offset], superblock
  call read_disk

  mov si, loading_msg
  call print

  jmp load_stage2

load_stage2:
  mov bx, loader_name
  ; Transform address from "0:offset" to "segment:0" fortmat
  mov dx, STAGE2_POSITION >> 4
  call load_file

  jmp protected_mode_enter ; Enter protected mode

protected_mode_enter:
  cli ; Turn off interrupts

  xor ax, ax ; Clear AX register
  mov ds, ax ; Set DS-register to 0
  mov es, ax
  mov fs, ax
  mov gs, ax

  ; Set multiboot info flags
  mov dword [boot_info + multiboot_info.flags], MULTIBOOT_FLAGS

  ; Set boot device
  mov [boot_info + multiboot_info.boot_device], dl

  ; Enable a20 line
  call enable_a20
  ; Install Global Descriptor Table
  call install_gdt

  sti

  xor	eax, eax
  xor ebx, ebx

  ; Get total physical memory
  call get_memory_size

  mov	word [boot_info + multiboot_info.memory_hi], bx
  mov	word [boot_info + multiboot_info.memory_lo], ax

  ; Get memory map
  mov di, 0x1000
  mov word [boot_info + multiboot_info.mmap_addr], di
  call get_memory_map
  mov word [boot_info + multiboot_info.mmap_length], ax

  ; Enter Protected Mode
  cli
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  jmp GDT_CODE_OFFSET:protected_mode

drive db 0
loader_name db "stage2.bin", NULL
loading_msg db "Loading Stage 2...", NEWLINE, RETURN, NULL

%include "src/bootloader/lib/stdio.inc"
%include "src/bootloader/lib/ext2.inc"
%include "src/bootloader/stage1/a20.inc"
%include "src/bootloader/stage1/memory.inc"
%include "src/bootloader/stage1/multiboot_info.inc"
%include "src/bootloader/stage1/gdt.inc"

[bits 32]

protected_mode:
  xor eax, eax

  mov ax, GDT_DATA_OFFSET ; Save data segment identifier
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov fs, ax
  mov gs, ax

  mov ebp, STACK_POINTER ; Move temp stack pointer to 0x90000
  mov esp, ebp

  mov edx, STAGE2_POSITION
  lea ecx, [edx]
  mov	ebx, [boot_info]

  ; Push multiboot info pointer on the stack
  push dword boot_info
  ; Call `stage2_main(multiboot_info)`
  call ecx

; Memory location where ext2 superblock will be loaded
superblock:
