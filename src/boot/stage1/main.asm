[bits 16]

%include "src/boot/lib/constants.inc"

%define MULTIBOOT_FLAGS_MEM 1
%define MULTIBOOT_FLAGS_BOOTDEVICE 1 << 2
%define MULTIBOOT_FLAGS_MMAP 1 << 7
%define MULTIBOOT_FLAGS_VBE 1 << 12

%define KERNEL_POSITION 0x8000 ; Loaded Stage 2 position
%define STACK_POINTER 0x300000 ; The initial stack pointer for Stage 2 loader

%define GDT_CODE_OFFSET 0x8 ; Offset to GDT code segment
%define GDT_DATA_OFFSET 0x10 ; Offset to GDT data segment

%define DISPLAY_RES_X 1024
%define DISPLAY_RES_Y 768
%define DISPLAY_BPP 32

%define BASE_SECTOR 63

global _start

_start:
  mov [boot_drive], dl

  ; Load ext2 superblock
  mov dword [packet + DAP.lba_lo], 2
  mov dword [packet + DAP.dest_offset], superblock
  call read_disk

  mov si, loading_msg
  call print

  jmp load_kernel

load_kernel:
  mov si, kernel_file_name
  call print

  mov bx, kernel_file_name
  ; Transform address from "0:offset" to "segment:0" format
  mov dx, KERNEL_POSITION >> 4
  call load_file

  mov si, debug_msg
  call print

  jmp protected_mode_enter ; Enter protected mode

protected_mode_enter:
  cli ; Turn off interrupts

  xor ax, ax ; Clear AX register
  mov ds, ax ; Set DS-register to 0
  mov es, ax
  mov fs, ax
  mov gs, ax

  ; Set multiboot info flags
  mov dword [boot_info + multiboot_info.flags], MULTIBOOT_FLAGS_MEM | MULTIBOOT_FLAGS_BOOTDEVICE | MULTIBOOT_FLAGS_MMAP | MULTIBOOT_FLAGS_VBE

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

  ; set VBE mode
  ; mov ax, DISPLAY_RES_X
  ; mov bx, DISPLAY_RES_Y
  ; mov cl, DISPLAY_BPP
  ; call vbe_set_mode
  ; mov word [boot_info + multiboot_info.vbe_mode_info], vbe_screen

  ; Enter Protected Mode
  cli
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  jmp GDT_CODE_OFFSET:protected_mode

debug_msg db "DEBUG", NEWLINE, RETURN, NULL
boot_drive db 0
loading_msg db "Loading kernel...", NEWLINE, RETURN, NULL
kernel_file_name db "kernel.bin", NULL

%include "src/boot/lib/ext2.inc"
%include "src/boot/lib/stdio.inc"
%include "src/boot/stage1/a20.inc"
%include "src/boot/stage1/memory.inc"
%include "src/boot/stage1/multiboot_info.inc"
%include "src/boot/stage1/vesa.inc"
%include "src/boot/stage1/gdt.inc"

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

  mov edx, KERNEL_POSITION
  lea ecx, [edx]
  mov	ebx, [boot_info]

  ; Push multiboot info pointer on the stack
  push dword boot_info
  ; Call `stage2_main(multiboot_info)`
  ; call ecx

  jmp $

; Memory location where ext2 superblock will be loaded
superblock:
