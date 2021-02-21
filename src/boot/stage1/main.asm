[bits 16]

%include "src/boot/lib/constants.inc"

%define MULTIBOOT_FLAGS_MEM 1
%define MULTIBOOT_FLAGS_BOOTDEVICE 1 << 2
%define MULTIBOOT_FLAGS_MMAP 1 << 7
%define MULTIBOOT_FLAGS_VBE 1 << 12

%define KERNEL_BINARY_POSITION 0x8000 ; Loaded Stage 2 position
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
  mov dx, KERNEL_BINARY_POSITION >> 4
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

%include "src/boot/stage1/terminal.inc"

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

  mov esi, KERNEL_BINARY_POSITION
  call load_elf

  jmp $

struc elf32_ehdr
  .e_ident resd 4 ; Magic number and other info
  .e_type resw 1 ; Object file type
  .e_machine resw 1 ; Architecture
  .e_version resd 1 ; Object file version
  .e_entry resd 1 ; Entry point virtual address
  .e_phoff resd 1 ; Program header table file offset
  .e_shoff resd 1 ; Section header table file offset
  .e_flags resd 1 ; Processor-specific flags
  .e_ehsize resw 1 ; ELF header size in bytes
  .e_phentsize resw 1 ; Program header table entry size
  .e_phnum resw 1 ; Program header table entry count
  .e_shentsize resw 1 ; Section header table entry size
  .e_shnum resw 1 ; Section header table entry count
  .e_shstrndx resw 1 ; Section header string table index
endstruc

struc elf32_phdr
  .p_type resd 1
  .p_offset resd 1 ; offset from beginning of file
  .p_vaddr resd 1
  .p_paddr resd 1
  .p_filesz resd 1 ; bytes of segment in file
  .p_memsz resd 1 ; bytes of segment in memory
  .p_flags resd 1
  .p_align resd 1
endstruc

ELF_MAGIC equ 0x464C457F

; Loads elf file into the memory at specified address.
; IN = esi: pointer to memory where file binary is loaded
; OUT = none
load_elf:
  pusha

  mov edi, esi

  mov esi, loading_kernel
  call terminal_write
  mov edx, edi
  call terminal_write_hex
  call terminal_write_nl

  mov edx, [edi + elf32_ehdr.e_ident]
  cmp edx, ELF_MAGIC
  jne .invalid_magic

  mov esi, elf_ok_magic
  call terminal_write_success

  ; print: data
  mov esi, data
  call terminal_write
  mov edx, edi
  call terminal_write_hex_line

  ; elf32_phdr *phdr = (elf32_phdr *)((uint32_t)data + ehdr->e_phoff);
  mov ebx, [edi + elf32_ehdr.e_phoff]
  add ebx, edi

  ; print: phdr
  mov esi, phdr
  call terminal_write
  mov edx, ebx
  call terminal_write_hex_line

  ; elf32_phdr *last_phdr = (elf32_phdr *)((uint32_t)phdr + (ehdr->e_phentsize * ehdr->e_phnum));
  movzx eax, word [edi + elf32_ehdr.e_phentsize]
  mul word [edi + elf32_ehdr.e_phnum]
  mov ecx, eax
  add ecx, ebx

  ; print: last_phdr
  mov esi, last_phdr
  call terminal_write
  mov edx, ecx
  call terminal_write_hex_line

  mov eax, ebx
  .loop:
    mov esi, dashes
    call terminal_write_line

    mov esi, phdr
    call terminal_write
    mov edx, eax
    call terminal_write_hex_line

    mov esi, phdr_p_paddr
    call terminal_write
    mov edx, [eax + elf32_phdr.p_paddr]
    call terminal_write_hex_line

    mov esi, dest
    call terminal_write
    mov edx, [eax + elf32_phdr.p_paddr]
    call terminal_write_hex_line

    mov esi, src
    call terminal_write
    mov edx, [eax + elf32_phdr.p_offset]
    add edx, edi
    call terminal_write_hex_line

    mov esi, len
    call terminal_write
    mov edx, [eax + elf32_phdr.p_filesz]
    call terminal_write_hex_line

    push ecx
    push edi

    ; copy to memory
    mov ecx, [eax + elf32_phdr.p_filesz] ; len
    mov esi, [eax + elf32_phdr.p_offset]
    add esi, edi ; src
    mov edi, [eax + elf32_phdr.p_paddr] ; dest
    rep movsb

    pop edi
    pop ecx

    add eax, 0x20

    mov esi, dashes
    call terminal_write_line

    cmp eax, ecx
    jl .loop ; loop end

    mov esi, entry
    call terminal_write
    mov edx, [edi + elf32_ehdr.e_entry]
    call terminal_write_hex_line

    jmp .done

  .invalid_magic:
    mov esi, elf_invalid_magic
    call terminal_write_error

    jmp $

  .done:
    mov	ebx, [boot_info]
    mov ecx, 0x2BADB002

    ; Call kernel
    mov eax, [edi + elf32_ehdr.e_entry]
    call eax

    jmp $

    popa
    ret

loading_kernel db "Loading kernel elf file from: ", NULL
elf_invalid_magic db "Invalid ELF magic number", NULL
elf_ok_magic db "ELF magic number OK", NULL
debug db "DEBUG", NULL

data db "data: ", NULL
phdr db "phdr: ", NULL
last_phdr db "last_phdr: ", NULL
dashes db "-------", NULL
phdr_p_paddr db "phdr->p_paddr: ", NULL
dest db "dest - phdr->p_paddr: ", NULL
src db "src - data + phdr->p_offset: ", NULL
len db "len - phdr->p_filesz: ", NULL
entry db "ehdr->e_entry: ", NULL

; Memory location where ext2 superblock will be loaded
superblock:
