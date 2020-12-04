;==============================================================================
; LBA DATA PACKET
PACKET:
			  db	0x10 ; packet size (16 bytes)
			  db	0	; always 0
count:	dw	4	; number of sectors to transfer
dest:		dw	0	; destination offset (0:7c00)
			  dw	0	; destination segment
lba:		dd	1	; put the lba # to read in this spot
			  dd	0	; more storage bytes only for big lba's ( > 4 bytes )
;==============================================================================

read_disk:
	mov esi, PACKET	; address of "disk address packet"
	mov ah, 0x42 ; extended read
	mov dl, [drive]	; drive number 0 (OR the drive # with 0x80)
	int 0x13
	jc error
	ret
