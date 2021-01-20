%ifndef EXT2_INC
%define EXT2_INC

%include "src/bootloader/lib/constants.inc"
%include "src/bootloader/lib/stdio.inc"
%include "src/bootloader/lib/string.inc"

%define EXT2_SB_SIZE 0x400 ; Superblock size
%define EXT2_SIG_OFFSET 0x38 ; Signature offset in superblock
%define EXT2_TABLE_OFFSET 0x8 ; Inode table offset after superblock
%define EXT2_INODE_TABLE_LOC 0x1000 ; New inode table location in memory
%define EXT2_ROOT_INODE 0x2 ; Root inode
%define EXT2_INODE_SIZE 0x80 ; Single inode size
%define EXT2_GET_ADDRESS(inode) (EXT2_INODE_TABLE_LOC + (inode - 1) * EXT2_INODE_SIZE)
%define EXT2_COUNT_OFFSET 0x1c ; Inode offset of number of data blocks
%define EXT2_POINTER_OFFSET 0x28 ; Inode offset of first data pointer
%define EXT2_IND_POINTER_OFFSET 0x2c ; Inode offset of singly indirect data pointer
%define EXT2_DIRECT_POINTER_COUNT 0xc ; Direct pointer count
%define EXT2_ENTRY_LENGTH_OFFSET 0x4 ; Dirent offset of entry length
%define EXT2_FILENAME_OFFSET 0x8 ; Dirent offset of filename
%define EXT2_INODE_OFFSET 0 ; Dirent offset of inode number
%define EXT2_SIG 0xef53 ; Signature

; Load file from ext2 file system.
; bx = file name
; dx = destination address to load the file to
load_file:
  push dx
  push bx

  mov ax, [superblock + EXT2_SIG_OFFSET] ; EXT2_MAGIC
  cmp ax, EXT2_SIG
  jne ext2_error ; Not a valid EXT2 disk

  mov si, ext2_success
  call print

  mov ax, [superblock + EXT2_SB_SIZE + EXT2_TABLE_OFFSET] ; Block_Group_Descriptor->inode_table
  shl ax, 1 ; Multiply ax by 2
  mov [packet + DAP.lba_lo], ax ; Which sector do we read?
  mov word [packet + DAP.dest_offset], EXT2_INODE_TABLE_LOC
  call read_disk

  ; Load root directory
  mov bx, EXT2_GET_ADDRESS(EXT2_ROOT_INODE) ; First block
  mov ax, [bx + EXT2_POINTER_OFFSET] ; Address of first block pointer
  shl ax, 1 ; Multiply ax by 2
  mov [packet + DAP.lba_lo], ax
  mov word [packet + DAP.dest_offset], 0x3500
  call read_disk
  pop dx ; Pop file name pointer into dx register

.search_loop:
  lea si, [bx + EXT2_FILENAME_OFFSET] ; First comparison string
  mov di, dx
  call strlen
  mov cx, ax
  rep cmpsb ; Compare strings
  je .found ; File found!
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

  pop dx
  mov [packet + DAP.dest_segment], dx
  mov word [packet + DAP.dest_offset], 0

  mov bx, 0
  call inode_load

  ret

; We enter the loop with:
; bx = inode pointer
; cx = # of sectors to read (2 per block)
; di = address of first block pointer
; No support for indirect pointers.
inode_load:
  mov ax, [di] ; Set ax = block pointer
  shl ax, 1 ; Multiply ax by 2

  mov [packet + DAP.lba_lo], ax
  mov [packet + DAP.dest_offset], bx

  call read_disk

  add bx, 0x400 ; 1 kb increase
  add di, 0x4	; Move to next block pointer
  sub cx, 0x2	; Read two blocks
  jnz inode_load
  ret

ext2_error:
  mov si, ext2_error_msg
  call print
  jmp $

found_msg db "File found...", NEWLINE, RETURN, NULL
ext2_success db "EXT2 Magic Header Good...", NEWLINE, RETURN, NULL
ext2_error_msg db "EXT2 superblock not found", NEWLINE, RETURN, NULL

%endif
