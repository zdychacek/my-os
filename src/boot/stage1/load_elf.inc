%ifndef LOAD_ELF_INC
%define LOAD_ELF_INC

%include "src/boot/stage1/terminal.inc"

ELF_MAGIC equ 0x464C457F

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

  mov ebx, [edi + elf32_ehdr.e_phoff]
  add ebx, edi

  movzx eax, word [edi + elf32_ehdr.e_phentsize]
  mul word [edi + elf32_ehdr.e_phnum]
  mov ecx, eax
  add ecx, ebx

  mov eax, ebx
  .loop:
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

    cmp eax, ecx
    jl .loop ; loop end

    ; print kernel entry point address
    mov esi, kernel_entry_address
    call terminal_write
    mov edx, [edi + elf32_ehdr.e_entry]
    call terminal_write_hex_line

    jmp .done

  .invalid_magic:
    mov esi, elf_invalid_magic
    call terminal_write_error

    jmp $

  .done:
    mov	ebx, boot_info
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
kernel_entry_address db "Kernel entry point address: ", NULL

%endif
