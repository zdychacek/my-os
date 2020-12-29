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

; GDT constants
%define GDT_CODE_OFFSET 0x08 ; Offset to GDT code segment
%define GDT_DATA_OFFSET 0x10 ; Offset to GDT data segment

%define REAL_STACK_BASE 0x7c00 ; Position stack base right before the bootloader.
%define STACK_POINTER 0x00300000 ; The initial stack pointer for Stage 2 loader
%define STAGE2_POSITION 0x00050000 ; Loaded Stage 2 position

; Multiboot flags
%define MULTIBOOT_FLAGS_MEM 1
%define MULTIBOOT_FLAGS_BOOTDEVICE 0b10
%define MULTIBOOT_FLAGS_MMAP 0b100000
%define MULTIBOOT_FLAGS (MULTIBOOT_FLAGS_MEM | MULTIBOOT_FLAGS_BOOTDEVICE | MULTIBOOT_FLAGS_MMAP)
