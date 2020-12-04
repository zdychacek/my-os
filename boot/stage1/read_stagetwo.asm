; We enter the loop with:
;	bx = inode pointer
; 	cx = # of sectors to read (2 per block)
;	di = address of first block pointer
; No support for indirect pointers. So keep
; stage2
read_stagetwo:
	xor ax, ax		; clear ax
	mov dx, ax		; clear dx
	mov ax, [di]	; set ax = block pointer
	mov dx, 2		; multiply by 2 for sectors
	mul dx			; ax = dx * ax

	mov [lba], ax
	mov [dest], bx

	call read_disk

	add bx, 0x400	; increase by 1 kb
	add di, 0x4		; move to next block pointer
	sub cx, 0x2		; reading two blocks
	jnz read_stagetwo
	ret

	nop
	hlt
