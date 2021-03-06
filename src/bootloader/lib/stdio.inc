%ifndef STDIO_INC
%define STDIO_INC

%include "src/bootloader/lib/constants.inc"

%define DISK_INT 0x13 ; Disk BIOS Interrupt
%define VIDEO_OUT 0xe ; Teletype output command
%define DISK_READ 0x42 ; Disk extended read command
%define DISK_EXT_CHECK 0x41 ; Disk extension check command
%define DISK_EXT_CHECK_SIG1 0x55aa ; First extension check signature
%define DISK_EXT_CHECK_SIG2 0xaa55 ; Second extension check signature

print:
  lodsb
  or al, al ; test for NULL termination
  jz .printdone
  mov ah, VIDEO_OUT
  int VIDEO_INT
  jmp print
.printdone:
  ret

; Disk Address Packet structure
struc DAP
  .packet_size resb	1
  .unused resb 1
  .count resw 1
  .dest_offset resw 1
  .dest_segment resw 1
  .lba_lo resd 1
  .lba_hi resd 1
endstruc

packet: istruc DAP
  at DAP.packet_size, db 0x10
  at DAP.unused, db 0
  at DAP.count, dw 4
  at DAP.dest_offset, dw 0
  at DAP.dest_segment, dw 0
  at DAP.lba_lo, dd 1
  at DAP.lba_hi, dd 0
iend

; Read blocks from disk.
read_disk:
  pusha

  mov esi, packet ; address of "disk address packet"
  mov ah, DISK_READ ; extended read
  mov dl, [drive] ; drive number 0 (OR the drive # with 0x80)
  int DISK_INT
  jc disk_error

  popa
  ret

; Check LBA support
check_lba_support:
  mov ah, DISK_EXT_CHECK
  mov bx, DISK_EXT_CHECK_SIG1
  int DISK_INT
  jc lba_error
  cmp bx, DISK_EXT_CHECK_SIG2
  jnz lba_error
  ret

; Errors
disk_error:
  mov si, disk_error_msg
  call print
  jmp $

lba_error:
  mov si, lba_error_msg
  call print
  jmp $

disk_error_msg db "Disk error!", NEWLINE, RETURN, NULL
lba_error_msg db "LBA error!", NEWLINE, RETURN, NULL

%endif
