%include "src/boot/lib/constants.inc"

; TODO:
%define BASE_SECTOR 0

[bits 16]

  jmp start
start:
  cli                         ; We do not want to be interrupted
  xor ax, ax                  ; 0 AX
  mov ds, ax                  ; Set Data Segment to 0
  mov es, ax                  ; Set Extra Segment to 0
  mov ss, ax                  ; Set Stack Segment to 0
  mov sp, ax                  ; Set Stack Pointer to 0
  mov sp, 0x7C00
  .relocate:
    mov cx, 0x0100            ; 256 WORDs in MBR
    mov si, 0x7C00            ; Current MBR Address
    mov di, 0x0600            ; New MBR Address
    rep movsw                 ; Copy MBR
  jmp 0:low_start             ; Jump to new Address

low_start:
  mov [boot_drive], dl ; BIOS stores drive # in dl

  mov si, starting_msg
  call print

  mov dword [packet + DAP.lba_lo], 1
  mov dword [packet + DAP.count], 1
  mov dword [packet + DAP.dest_offset], mbr_end
  call read_disk

  jmp mbr_end

starting_msg db "Starting...", NEWLINE, RETURN, NULL

%include "src/boot/lib/stdio.inc"

times 218 - ($-$$) db 0       ; Pad for disk time stamp

disk_time_stamp times 8 db 0    ; Disk Time Stamp

boot_drive db 0                ; Our Drive Number Variable
partition_offset dw 0                    ; Our Partition Table Entry Offset

times 0x1b4 - ($-$$) db 0     ; Pad For MBR Partition Table

UID times 10 db 0             ; Unique Disk ID
partition_table_1 times 16 db 0
partition_table_2 times 16 db 0
partition_table_3 times 16 db 0
partition_table_4 times 16 db 0

dw 0xaa55

; ====== start of second sector
mbr_end:
  sti

  mov si, checking_lba_msg
  call print

  ; TODO: move to first sector
  ; check for LBA support
  call check_lba_support

  mov si, checking_lba_ok_msg
  call print

.jump_to_VBR:
  check_partitions:           ; Check Partition Table For Bootable Partition
    mov bx, partition_table_1               ; Base = Partition Table Entry 1
    mov cx, 4                 ; There are 4 Partition Table Entries
    .loop:
      mov al, byte [bx]       ; Get Boot indicator bit flag
      test al, 0x80           ; Check For Active Bit
      jnz .found              ; We Found an Active Partition
      add bx, 0x10            ; Partition Table Entry is 16 Bytes
      dec cx                  ; Decrement Counter
      jnz .loop               ; Loop
    jmp error                 ; ERROR!
    .found:
      mov word [partition_offset], bx    ; Save Offset
      add bx, 8               ; Increment Base to LBA Address
  .read_VBR:
    mov ax, [bx]
    mov word [packet + DAP.lba_lo], ax
    mov dword [packet + DAP.count], 1
    mov dword [packet + DAP.dest_offset], 0x7c00
    call read_disk

  .jump_to_VBR:
    cmp word [0x7dfe], 0xaa55 ; Check Boot Signature
    jne error                 ; Error if not Boot Signature
    mov si, word [partition_offset]      ; Set DS:SI to Partition Table Entry
    mov dl, byte [boot_drive] ; Set DL to Drive Number
    jmp 0x7c00                ; Jump To VBR

  error:
    mov si, error_msg
    call print
    jmp $

  ; ====== messages
  checking_lba_msg db "Checking LBA support", NULL
  checking_lba_ok_msg db "...OK", NEWLINE, RETURN, NULL
  error_msg db "No bootable partition found :(", NEWLINE, RETURN, NULL

times 1024 - ($-$$) db 0
