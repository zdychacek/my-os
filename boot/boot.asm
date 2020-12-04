;	MEMORY MAP
; 0x0400 : 0x0500 - memory map info
; 0x2000 : 0x2??? - VESA controller info
; 0x3000 :
; 0x5000 : 0x7200 Stage2 bootloader
; 0x7C00 : 0x8000 Stage1/1.5 bootloader

[BITS 16]
[ORG 0x7C00]

GLOBAL boot

; Stage1 is responsible for loading Stage1.5, and mapping video modes
boot:
	and dl, 0x80 ; 0x80 = hdd, 0x81 = hdd2
	jz error

	mov [drive], dl ; BIOS stores drive # in dl

	mov bx, stage_oneandhalf ; Load into address of Stage1.5
	mov [dest], bx
	call read_disk

	jmp stage_oneandhalf

error:
	mov esi, ext2_error
	call print
	jmp $

%include "boot/stage1/read_disk.asm"
%include "boot/stage1/print.asm"

ext2_success      db 13, "EXT2 Magic Header Good!", 10, 0
ext2_error			  db 13, "EXT2 superblock not found", 10, 0
stageonepointfive db 13, "Stage1.5 loaded!", 10, 0
drive				      db 0

times 510-($-$$) db 0           ; Fill up the file with zeros
dw 0AA55h                       ; Last 2 bytes = Boot sector identifyer

;==============================================================================
; END 	LBA SECTOR 0.
;
; We are now out of the zone loaded by the BIOS
; However, Stage1 contains some useful functions that we can continue
; to use, since Stage1.5 is directly loaded at the end of stage1 (0x7E00)
;
; Stage1.5 mainly focuses on parsing ext2 data to find the blocks used for
; inode #5, which is reserved by ext2 specific for bootloaders - and we have
; placed the Stage2 loaded there. Memory mapping function is also placed here
;
; BEGIN 	LBA SECTOR 1
;==============================================================================
%include "boot/stage1/stage_oneandhalf.asm"
%include "boot/stage1/memory_map.asm"
%include "boot/stage1/read_stagetwo.asm"
%include "boot/stage1/enter_pm.asm"
%include "boot/stage1/gdt.asm"

times 1024-($-$$) db 0

;==============================================================================
; END 	LBA SECTOR 1.
;
; We can use the end of the file for a convenient label
; Superblock starts at LBA 2, which is the end of this
; sector
;
; BEGIN 	LBA SECTOR 2
;==============================================================================
superblock:
