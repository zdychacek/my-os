; Boot constants
%define SECTOR_END 0xaa55 ; Bootsector end signature

; Characters
%define NEWLINE 0x0A ; Newline character (\n)
%define RETURN 0x0D ; Return character (\r)
%define NULL 0x00 ; NULL character (\0)

; Interrupts
%define VIDEO_INT 0x10 ; Video BIOS Interrupt
%define DISK_INT 0x13 ; Disk BIOS Interrupt
%define MISC_INT 0x15 ; Miscellaneous services BIOS Interrupt

; Video commands (VGA)
%define VIDEO_OUT 0x0e ; Teletype output command

; Disk commands
%define DISK_EXT_CHECK 0x41 ; Disk extension check command
%define DISK_EXT_CHECK_SIG1 0x55aa ; First extension check signature
%define DISK_EXT_CHECK_SIG2 0xaa55 ; Second extension check signature
%define DISK_ZERO 0x80 ; First disk
%define DISK_READ 0x42 ; Disk extended read command

; EXT2 constants
%define EXT2_SB_SIZE 0x400 ; Superblock size
%define EXT2_SIG_OFFSET 0x38 ; Signature offset in superblock
%define EXT2_TABLE_OFFSET 0x08 ; Inode table offset after superblock
%define EXT2_INODE_TABLE_LOC 0x1000 ; New inode table location in memory
%define EXT2_ROOT_INODE 0x02 ; Root inode
%define EXT2_INODE_SIZE 0x80 ; Single inode size
%define EXT2_GET_ADDRESS(inode) (EXT2_INODE_TABLE_LOC + (inode - 1) * EXT2_INODE_SIZE)
%define EXT2_COUNT_OFFSET 0x1c ; Inode offset of number of data blocks
%define EXT2_POINTER_OFFSET 0x28 ; Inode offset of first data pointer
%define EXT2_IND_POINTER_OFFSET 0x2c ; Inode offset of singly indirect data pointer
%define EXT2_DIRECT_POINTER_COUNT 0x0c ; Direct pointer count
%define EXT2_ENTRY_LENGTH_OFFSET 0x04 ; Dirent offset of entry length
%define EXT2_FILENAME_OFFSET 0x08 ; Dirent offset of filename
%define EXT2_INODE_OFFSET 0x00 ; Dirent offset of inode number
%define EXT2_SIG 0xef53 ; Signature

; A20 constants
%define A20_GATE 0x92 ; Fast A20 gate
%define A20_ENABLED 0b10 ; Bit 1 defines whether A20 is enabled

; GDT constants (bitmap)
%define GDT_MAX_LIMIT 0xffff ; I just use the max limit lel
%define GDT_PRESENT 0b10000000 ; Is present
%define GDT_RING3 0b01100000 ; Privilege level 3
%define GDT_DESCRIPTOR 0b00010000 ; Descriptor type, set for code/data
%define GDT_EXECUTABLE 0b00001000 ; Can be executed
%define GDT_READWRITE 0b00000010 ; Read/write access for code/data
%define GDT_ACCESSED 0b00000001 ; Whether segment is accessed
%define GDT_GRANULARITY (0x80 | 0x00) ; Page granularity (4KiB)
%define GDT_SIZE (0x40 | 0x00) ; Use 32 bit selectors
%define GDT_CODE_OFFSET 0x08 ; Offset to GDT code segment
%define GDT_DATA_OFFSET 0x10 ; Offset to GDT data segment

%define REAL_STACK_BASE 0x7c00 ; Position stack base right before the bootloader.
%define STACK_POINTER 0x00900000 ; The initial stack pointer for Stage 2 loader
%define STAGE2_POSITION 0x00050000 ; Loaded Stage 2 position

[bits 16]

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
  mov [dest], bx
  call read_disk

  jmp stage_oneandhalf

;==============================================================================
; LBA DATA PACKET
PACKET:
        db	0x10 ; packet size (16 bytes)
        db	0 ; always 0
count:	dw	4 ; number of sectors to transfer
dest:		dw	0 ; destination offset (0:7c00)
        dw	0 ; destination segment
lba:		dd	1 ; put the lba # to read in this spot
        dd	0 ; more storage bytes only for big lba's ( > 4 bytes )
;==============================================================================

read_disk:
  mov esi, PACKET ; address of "disk address packet"
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
  mov [lba], ax ; Which sector do we read?
  mov bx, EXT2_INODE_TABLE_LOC; copy data to 0x1000
  mov [dest], bx
  call read_disk

  ; Load root directory
  mov bx, EXT2_GET_ADDRESS(EXT2_ROOT_INODE) ; First block
  mov ax, [bx + EXT2_POINTER_OFFSET] ; Address of first block pointer
  shl ax, 1 ; Multiply ax by 2
  mov [lba], ax
  mov bx, 0x3500 ; Load to this address
  mov [dest], bx
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
  mov [dest + 2], bx
  mov bx, 0	; where inode5 will be loaded. 0xF0000
  mov [dest], bx
  call inode_load

  ; ================ Prepare to call the memory mapping function ================
  xor eax, eax ; Clear EAX
  mov es, eax	; Clear ES
  mov edi, 0x400 ; DI = 0x400. ES:DI => 0x00000400
  push edi ; Push start of memory map

  call memory_map
  push edi ; Push end of memory map
  jmp protected_mode_enter ; Enter protected mode

; We enter the loop with:
;	bx = inode pointer
; cx = # of sectors to read (2 per block)
;	di = address of first block pointer
; No support for indirect pointers.
inode_load:
  mov ax, [di] ; Set ax = block pointer
  shl ax, 1 ; Multiply ax by 2

  mov [lba], ax
  mov [dest], bx

  call read_disk

  add bx, 0x400	; 1 kb increase
  add di, 0x4	; Move to next block pointer
  sub cx, 0x2	; Read two blocks
  jnz inode_load
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

ext2_error:
  mov si, ext2_error_msg
  call print
  jmp $

; Bios function INT 15h, AX=E820h
; EBX must contain 0 on first call, and remain unchanged
; on subsequent calls until it is zero again
; Code adapted from OSDEV wiki
; Memory map is 24 byte struct placed at [ES:DI]
memory_map:
  xor ebx, ebx ; ebx must be 0 to start
  mov edx, 0x0534D4150 ; Place "SMAP" into edx
  mov eax, 0xe820
  mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
  mov ecx, 24	; ask for 24 bytes
  int MISC_INT
  jc short .fail ; carry set on first call means "unsupported function"

  mov edx, 0x0534D4150
  cmp eax, edx ; on success, eax must have been set to "SMAP"
  jne short .fail

  test ebx, ebx ; ebx = 0 implies list is only 1 entry long (worthless)
  je short .fail

  jmp short .loop

.e820lp:
  mov eax, 0xe820	; eax, ecx get trashed on every int 0x15 call
  mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
  mov ecx, 24 ; ask for 24 bytes again
  int MISC_INT
  jc short .done ; carry set means "end of list already reached"
  mov edx, 0x0534D4150 ; repair potentially trashed register
.loop:
  jcxz .skip ; skip any 0 length entries
  cmp cl, 20 ; got a 24 byte ACPI 3.X response?
  jbe short .notext
  test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
  je short .skip
.notext:
  mov ecx, [es:di + 8] ; get lower uint32_t of memory region length
  or ecx, [es:di + 12] ; "or" it with upper uint32_t to test for zero
  jz .skip ; if length uint64_t is 0, skip entry
  add di, 24

.skip:
  test ebx, ebx	; if ebx resets to 0, list is complete
  jne short .e820lp
.done:
  clc	; there is "jc" on end of list to this point, so the carry must be cleared
  ret
.fail:
  stc	; "function unsupported" error exit
  ret

align 16

mem_info:
  dd 0
  dd 0

protected_mode_enter:
  cli ; Turn off interrupts

  ; Enable a20 line
  in al, A20_GATE
  or al, A20_ENABLED
  out A20_GATE, al

  xor ax, ax ; Clear AX register
  mov ds, ax ; Set DS-register to 0
  mov es, ax
  mov fs, ax
  mov gs, ax

  lgdt [gdt_desc] ; Load the Global Descriptor Table

  ; Enter Protected Mode
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  jmp GDT_CODE_OFFSET:protected_mode ; Jump to code segment, offset kernel_segments

[bits 32]

protected_mode:
  pop ecx	; End of memory map
  mov [mem_info+4], ecx
  pop ecx
  mov [mem_info], ecx

  xor eax, eax

  mov ax, GDT_DATA_OFFSET ; Save data segment identifier
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov fs, ax
  mov gs, ax

  mov ebp, STACK_POINTER ; Move temp stack pointer to 090000h
  mov esp, ebp

  mov eax, mem_info
  push eax
  mov edx, STAGE2_POSITION
  lea eax, [edx]
  call eax ; stage2_main(mem_info)

; GLOBAL DESCRIPTOR TABLE
align 32

gdt: ; Address for the GDT

gdt_null:
  dd 0
  dd 0

gdt_kernel_code:
  dw GDT_MAX_LIMIT ; Limit
  dw 0 ; First base
  db 0 ; Second base
  db (GDT_PRESENT | GDT_DESCRIPTOR | GDT_EXECUTABLE | GDT_READWRITE) ; Configuration
  db (GDT_GRANULARITY | GDT_SIZE) ; Flags
  db 0 ; Third base

gdt_kernel_data:
  dw GDT_MAX_LIMIT ; Limit
  dw 0 ; First base
  db 0 ; Second base
  db (GDT_PRESENT | GDT_DESCRIPTOR | GDT_READWRITE) ; Configuration
  db (GDT_GRANULARITY | GDT_SIZE) ; Flags
  db 0 ; Third base

gdt_end: ; Used to calculate the size of the GDT

gdt_desc: ; The GDT descriptor
  dw gdt_end - gdt - 1 ; Limit (size)
  dd gdt ; Address of the GDT

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
