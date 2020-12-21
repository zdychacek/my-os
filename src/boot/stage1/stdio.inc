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
	at DAP.packet_size,	db 0x10
	at DAP.unused, db 0
	at DAP.count, dw 4
	at DAP.dest_offset, dw 0
	at DAP.dest_segment, dw 0
  at DAP.lba_lo, dd 1
  at DAP.lba_hi, dd 0
iend

read_disk:
  mov esi, packet ; address of "disk address packet"
  mov ah, DISK_READ ; extended read
  mov dl, [drive] ; drive number 0 (OR the drive # with 0x80)
  int DISK_INT
  jc disk_error
  ret

print:
  lodsb
  or al, al ; test for NULL termination
  jz .printdone
  mov ah, VIDEO_OUT
  int VIDEO_INT
  jmp print
.printdone:
  ret