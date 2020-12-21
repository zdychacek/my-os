[bits 16]

%include "src/boot/stage1/constants.inc"

global _start

; Stage 1 is responsible for loading Stage 1.5
_start:
  mov bp, REAL_STACK_BASE
  mov sp, bp

  ; Check LBA support
  mov ah, DISK_EXT_CHECK
  mov bx, DISK_EXT_CHECK_SIG1
  int DISK_INT
  jc lba_error
  cmp bx, DISK_EXT_CHECK_SIG2
  jnz lba_error

  ; Check HDD
  and dl, DISK_ZERO ; 0x80 = hdd, 0x81 = hdd2
  jz disk_error
  mov [drive], dl ; BIOS stores drive # in dl

  ; Load Stage 1.5
  mov bx, stage_oneandhalf
  mov [packet + DAP.dest_offset], bx
  call read_disk

  jmp stage_oneandhalf

drive             db 0
loader_name       db "stage2.bin"
loader_name_len   equ $ - loader_name

; Messages
stageonepointfive db "Stage1.5 loaded!", NEWLINE, RETURN, NULL
ext2_success      db "EXT2 Magic Header Good!", NEWLINE, RETURN, NULL
ext2_error_msg	  db "EXT2 superblock not found", NEWLINE, RETURN, NULL
disk_error_msg    db "Disk error!", NEWLINE, RETURN, NULL
lba_error_msg     db "LBA error!", NEWLINE, RETURN, NULL
found_msg         db "Found stage 2 binary!", NEWLINE, RETURN, NULL

%include "src/boot/stage1/stdio.inc"
%include "src/boot/stage1/a20.inc"
%include "src/boot/stage1/memory.inc"

; Errors
disk_error:
  mov si, disk_error_msg
  call print
  jmp $

lba_error:
  mov si, lba_error_msg
  call print
  jmp $

ext2_error:
  mov si, ext2_error_msg
  call print
  jmp $

times 510-($-$$) db 0 ; Fill up the file with zeros
dw SECTOR_END ; Last 2 bytes = Boot sector identifyer

;==============================================================================
; END 	LBA SECTOR 0.
;
; We are now out of the zone loaded by the BIOS
; However, Stage 1 contains some useful functions that we can continue
; to use, since Stage 1.5 is directly loaded at the end of stage1 (0x7E00)
;
; Stage 1.5 mainly focuses on parsing ext2 data to find the blocks used for
; Stage 2 binary. Memory mapping function is also placed here.
;
; BEGIN 	LBA SECTOR 1
;==============================================================================

; Stage 1.5 is responsible for loading Stage 2
stage_oneandhalf:
  mov esi, stageonepointfive
  call print

  mov ax, [superblock + EXT2_SIG_OFFSET] ; EXT2_MAGIC
  cmp ax, EXT2_SIG
  jne ext2_error ; Not a valid EXT2 disk

  mov si, ext2_success
  call print

  mov ax, [superblock + EXT2_SB_SIZE + EXT2_TABLE_OFFSET] ; Block_Group_Descriptor->inode_table
  shl ax, 1 ; Multiply ax by 2
  mov [packet + DAP.lba_lo], ax ; Which sector do we read?
  mov bx, EXT2_INODE_TABLE_LOC; copy data to 0x1000
  mov [packet + DAP.dest_offset], bx
  call read_disk

  ; Load root directory
  mov bx, EXT2_GET_ADDRESS(EXT2_ROOT_INODE) ; First block
  mov ax, [bx + EXT2_POINTER_OFFSET] ; Address of first block pointer
  shl ax, 1 ; Multiply ax by 2
  mov [packet + DAP.lba_lo], ax
  mov bx, 0x3500 ; Load to this address
  mov [packet + DAP.dest_offset], bx
  call read_disk

.search_loop:
  lea si, [bx + EXT2_FILENAME_OFFSET] ; First comparison string
  mov di, loader_name ; Second comparison string
  mov cx, loader_name_len ; String length
  rep cmpsb ; Compare strings
  je .found ; Found loader!
  add bx, EXT2_ENTRY_LENGTH_OFFSET ; Add dirent struct size
  jmp .search_loop ; Next dirent!

.found:
  mov si, found_msg
  call print ; Print success message
  mov ax, word [bx + EXT2_INODE_OFFSET] ; Get inode number from dirent
  ; Calculate address: (EXT2_INODE_TABLE_LOC + (inode - 1) * EXT2_INODE_SIZE)
  dec ax ; (inode - 1)
  mov cx, EXT2_INODE_SIZE ; Prepare for multiplication
  mul cx ; Multiply inode number
  mov bx, ax ; Transfer calculation
  add bx, EXT2_INODE_TABLE_LOC ; bx is at the start of the inode now!
  mov cx, [bx + EXT2_COUNT_OFFSET] ; Number of blocks for inode
  cmp cx, 0
  je disk_error
  lea di, [bx + EXT2_POINTER_OFFSET] ; address of first block pointer
  mov bx, 0x5000
  mov [packet + DAP.dest_segment], bx
  mov bx, 0	; where inode5 will be loaded
  mov [packet + DAP.dest_offset], bx
  call inode_load

  jmp protected_mode_enter ; Enter protected mode

; We enter the loop with:
;	bx = inode pointer
; cx = # of sectors to read (2 per block)
;	di = address of first block pointer
; No support for indirect pointers.
inode_load:
  mov ax, [di] ; Set ax = block pointer
  shl ax, 1 ; Multiply ax by 2

  mov [packet + DAP.lba_lo], ax
  mov [packet + DAP.dest_offset], bx

  call read_disk

  add bx, 0x400	; 1 kb increase
  add di, 0x4	; Move to next block pointer
  sub cx, 0x2	; Read two blocks
  jnz inode_load
  ret

protected_mode_enter:
  cli ; Turn off interrupts

  xor ax, ax ; Clear AX register
  mov ds, ax ; Set DS-register to 0
  mov es, ax
  mov fs, ax
  mov gs, ax

  ; Set multiboot info flags
  mov dword [boot_info + multiboot_info.flags], MULTIBOOT_FLAGS

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

  jmp GDT_CODE_OFFSET:protected_mode ; Jump to code segment, offset kernel_segments

[bits 32]

protected_mode:
  xor eax, eax

  mov ax, GDT_DATA_OFFSET ; Save data segment identifier
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov fs, ax
  mov gs, ax

  mov ebp, STACK_POINTER ; Move temp stack pointer to 090000h
  mov esp, ebp

  push dword boot_info

  mov edx, STAGE2_POSITION
  lea ecx, [edx]
  mov	ebx, [boot_info]
  call ecx ; stage2_main(multiboot_info)

%include "src/boot/stage1/gdt.inc"
%include "src/boot/stage1/multiboot_info.inc"

boot_info: istruc multiboot_info
  at multiboot_info.flags, dd 0
  at multiboot_info.memory_lo, dd 0
  at multiboot_info.memory_hi, dd 0
  at multiboot_info.boot_device, dd 0
  at multiboot_info.cmdLine, dd 0
  at multiboot_info.mods_count, dd 0
  at multiboot_info.mods_addr, dd 0
  at multiboot_info.syms0, dd 0
  at multiboot_info.syms1, dd 0
  at multiboot_info.syms2, dd 0
  at multiboot_info.mmap_length, dd 0
  at multiboot_info.mmap_addr, dd 0
  at multiboot_info.drives_length, dd 0
  at multiboot_info.drives_addr, dd 0
  at multiboot_info.config_table, dd 0
  at multiboot_info.bootloader_name, dd 0
  at multiboot_info.apm_table, dd 0
  at multiboot_info.vbe_control_info, dd 0
  at multiboot_info.vbe_mode_info, dw 0
  at multiboot_info.vbe_interface_seg, dw 0
  at multiboot_info.vbe_interface_off, dw 0
  at multiboot_info.vbe_interface_len, dw 0
iend

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
