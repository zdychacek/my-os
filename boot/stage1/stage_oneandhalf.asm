stage_oneandhalf:
	mov esi, stageonepointfive
	call print

	xor bx, bx

	mov ax, [superblock  +56]		; EXT2_MAGUC
	cmp ax, 0xEF53
	jne error 						; Not a valid EXT2 disk

	mov si, ext2_success
	call print

; We need to read the inode table entry for inode 5

	mov ax, [superblock  + 1024 + 8]	; Block_Group_Descriptor->inode_table
	mov cx, 2
	mul cx								; ax = cx * ax

	mov [lba], ax						; which sector do we read?

	mov ax, 2							; read 1024 bytes
	mov [count], ax

	mov bx, 0x1000						; copy data to 0x1000
	mov [dest], bx

	call read_disk

	xor bx, bx
	mov bx, 0x1200		; Inode 5 is 0x200 into the first block (index 4 * 128 bytes per inode)
	mov cx, [bx + 28]	; # of sectors for inode
	lea di, [bx + 40]	; address of first block pointer

	mov bx, 0x5000
	mov [dest+2], bx
	mov bx, 0		; where inode5 will be loaded. 0xF0000
	mov [dest], bx
	call read_stagetwo

; Prepare to call the memory mapping function
	xor eax, eax		; Clear EAX
	mov es, eax			; Clear ES
	mov edi, 0x400		; DI = 0x400. ES:DI => 0x00000400
	push edi			; Push start of memory map

	call memory_map
	push edi			; Push end of memory map
	jmp enter_pm 		; Enter protected mode
