[bits 16]

%include "src/bootloader/lib/constants.inc"

%define STAGE1_POSITION 0x5000
%define REAL_STACK_BASE 0x7c00 ; Position stack base right before the bootloader.
%define SECTOR_END 0xaa55 ; Bootsector end signature
%define DISK_ZERO 0x80 ; First disk

global _start

; Stage 1 is responsible for loading Stage 1
_start:
  mov bp, REAL_STACK_BASE
  mov sp, bp

  call check_lba_support

  ; Check HDD
  and dl, DISK_ZERO ; 0x80 = hdd, 0x81 = hdd2
  jz disk_error
  mov [drive], dl ; BIOS stores drive # in dl

  ; Load Stage 1
  mov dword [packet + DAP.lba_lo], 2
  mov dword [packet + DAP.dest_offset], superblock
  call read_disk

  mov si, loading_msg
  call print

  jmp load_stage1

; Stage 1 is responsible for loading Stage 2
load_stage1:
  mov bx, loader_name
  ; Transform address from "0:offset" to "segment:0" format
  mov dx, STAGE1_POSITION >> 4
  call load_file

  mov dx, [drive]
  jmp STAGE1_POSITION

drive db 0
loader_name db "stage1.bin", NULL
loading_msg db "Loading Stage 1...", NEWLINE, RETURN, NULL

%include "src/bootloader/lib/stdio.inc"
%include "src/bootloader/lib/ext2.inc"

times 510-($-$$) db 0 ; Fill up the file with zeros
dw SECTOR_END ; Last 2 bytes = Boot sector identifier

; Memory location where ext2 superblock will be loaded
superblock:
