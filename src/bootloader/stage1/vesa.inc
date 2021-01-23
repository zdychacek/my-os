%ifndef VESA_INC
%define VESA_INC

%include "src/bootloader/lib/constants.inc"

struc vbe_info_block_struct
  .signature resb	4
  .version resw 1
  .oem resd 1
  .capabilities resd 1
  .video_modes resd 1
  .video_memory resw 1
  .software_rev resw 1
  .vendor resd 1
  .product_name resd 1
  .product_rev resd 1
  .reserved resb 222
  .oem_data resb 256
endstruc

vbe_info_block: istruc vbe_info_block_struct
  at vbe_info_block_struct.signature, db "VBE2"
  at vbe_info_block_struct.version, dw 0
  at vbe_info_block_struct.oem, dd 0
  at vbe_info_block_struct.capabilities, dd 0
  at vbe_info_block_struct.video_modes, dd 0
  at vbe_info_block_struct.video_memory, dw 0
  at vbe_info_block_struct.software_rev, dw 0
  at vbe_info_block_struct.vendor, dd 0
  at vbe_info_block_struct.product_name, dd 0
  at vbe_info_block_struct.product_rev, dd 0
  at vbe_info_block_struct.reserved, times 222 db 0
  at vbe_info_block_struct.oem_data, times 256 db 0
iend

struc vbe_mode_info_block_struct
  .attributes resw 1 ; deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
  .window_a resb 1 ; deprecated
  .window_b resb 1  ; deprecated
  .granularity resw 1 ; deprecated ; used while calculating bank numbers
  .window_size resw 1
  .segment_a resw 1
  .segment_b resw 1
  .win_func_ptr resd 1 ; deprecated ; used to switch banks from protected mode without returning to real mode
  .pitch resw 1 ;	number of bytes per horizontal line
  .width resw 1 ; width in pixels
  .height resw 1 ; height in pixels
  .w_char resb 1 ; unused...
  .y_char resb 1 ; unused...
  .planes resb 1
  .bpp resb 1 ;	bits per pixel in this mode
  .banks resb 1 ; deprecated ; total number of banks in this mode
  .memory_model resb 1
  .bank_size resb 1 ; deprecated ; size of a bank, almost always 64 KB but may be 16 KB...
  .image_pages resb 1
  .reserved0 resb 1

  .red_mask resb 1
  .red_position resb 1
  .green_mask resb 1
  .green_position resb 1
  .blue_mask resb 1
  .blue_position resb 1
  .reserved_mask resb 1
  .reserved_position resb 1
  .direct_color_attributes resb 1

  .framebuffer resd 1 ; physical address of the linear frame buffer ; write here to draw to the screen
  .off_screen_mem_off resd 1
  .off_screen_mem_size resw 1 ;	size of memory in the framebuffer but not being displayed on the screen
  .reserved1 resb 206
endstruc

vbe_mode_info_block: istruc vbe_mode_info_block_struct
  at vbe_mode_info_block_struct.attributes, dw 0
  at vbe_mode_info_block_struct.window_a, db 0
  at vbe_mode_info_block_struct.window_b, db 0
  at vbe_mode_info_block_struct.granularity, dw 0
  at vbe_mode_info_block_struct.window_size, dw 0
  at vbe_mode_info_block_struct.segment_a, dw 0
  at vbe_mode_info_block_struct.segment_b, dw 0
  at vbe_mode_info_block_struct.win_func_ptr, dd 0
  at vbe_mode_info_block_struct.pitch, dw 0
  at vbe_mode_info_block_struct.width, dw 0
  at vbe_mode_info_block_struct.height, dw 0
  at vbe_mode_info_block_struct.w_char, db 0
  at vbe_mode_info_block_struct.y_char, db 0
  at vbe_mode_info_block_struct.planes, db 0
  at vbe_mode_info_block_struct.bpp, db 0
  at vbe_mode_info_block_struct.banks, db 0
  at vbe_mode_info_block_struct.memory_model, db 0
  at vbe_mode_info_block_struct.bank_size, db 0
  at vbe_mode_info_block_struct.image_pages, db 0
  at vbe_mode_info_block_struct.reserved0, db 0

  at vbe_mode_info_block_struct.red_mask, db 0
  at vbe_mode_info_block_struct.red_position, db 0
  at vbe_mode_info_block_struct.green_mask, db 0
  at vbe_mode_info_block_struct.green_position, db 0
  at vbe_mode_info_block_struct.blue_mask, db 0
  at vbe_mode_info_block_struct.blue_position, db 0
  at vbe_mode_info_block_struct.reserved_mask, db 0
  at vbe_mode_info_block_struct.reserved_position, db 0
  at vbe_mode_info_block_struct.direct_color_attributes, db 0

  at vbe_mode_info_block_struct.framebuffer, dd 0
  at vbe_mode_info_block_struct.off_screen_mem_off, dd 0
  at vbe_mode_info_block_struct.off_screen_mem_size, dw 0
  at vbe_mode_info_block_struct.reserved1, times 206 db 0
iend

struc vbe_screen_struct
  .width resw	1
  .height resw 1
  .framebuffer resd 1
  .pitch resw 1
  .bpp resb 1
  .bytes_per_pixel resw 1
endstruc

vbe_screen: istruc vbe_screen_struct
  at vbe_screen_struct.width, dw 0
  at vbe_screen_struct.height, dw 0
  at vbe_screen_struct.framebuffer, dd 0
  at vbe_screen_struct.pitch, dw 0
  at vbe_screen_struct.bpp, db 0
  at vbe_screen_struct.bytes_per_pixel, dw 0
iend

; vbe_set_mode:
; Sets a VESA mode
; In\	AX = Width
; In\	BX = Height
; In\	CL = Bits per pixel
; Out\	Width, height, bpp, physical buffer, all set in vbe_screen structure
vbe_set_mode:
  mov [.width], ax
  mov [.height], bx
  mov [.bpp], cl

  sti

  push es ; some VESA BIOSes destroy ES, or so I read
  mov ax, 0x4f00 ; get VBE BIOS info
  mov di, vbe_info_block
  int VIDEO_INT
  pop es

  cmp ax, 0x4f ; BIOS doesn't support VBE?
  jne .error

  mov ax, word [vbe_info_block + vbe_info_block_struct.video_modes]
  mov [.offset], ax
  mov ax, word [vbe_info_block + vbe_info_block_struct.video_modes + 2]
  mov [.segment], ax

  mov ax, [.segment]
  mov fs, ax
  mov si, [.offset]

.find_mode:
	mov dx, [fs:si]
	add si, 2
	mov [.offset], si
	mov [.mode], dx
	mov ax, 0
	mov fs, ax

	cmp word [.mode], 0xffff ; end of list?
	je .error

	push es
	mov ax, 0x4f01 ; get VBE mode info
	mov cx, [.mode]
	mov di, vbe_mode_info_block
	int VIDEO_INT
	pop es

	cmp ax, 0x4f
	jne .error

	mov ax, [.width]
	cmp ax, [vbe_mode_info_block + vbe_mode_info_block_struct.width]
	jne .next_mode

	mov ax, [.height]
	cmp ax, [vbe_mode_info_block + vbe_mode_info_block_struct.height]
	jne .next_mode

	mov al, [.bpp]
	cmp al, [vbe_mode_info_block + vbe_mode_info_block_struct.bpp]
	jne .next_mode

	; If we make it here, we've found the correct mode!
	mov ax, [.width]
	mov word [vbe_screen + vbe_screen_struct.width], ax

	mov ax, [.height]
	mov word [vbe_screen + vbe_screen_struct.height], ax

	mov eax, [vbe_mode_info_block + vbe_mode_info_block_struct.framebuffer]
	mov dword [vbe_screen + vbe_screen_struct.framebuffer], eax

	mov ax, [vbe_mode_info_block + vbe_mode_info_block_struct.pitch]
	mov word [vbe_screen + vbe_screen_struct.pitch], ax

	mov eax, 0
	mov al, [.bpp]
	mov byte [vbe_screen + vbe_screen_struct.bpp], al

	shr eax, 3
	mov dword [vbe_screen + vbe_screen_struct.bytes_per_pixel], eax

	; Set the mode
	push es
	mov ax, 0x4f02
	mov bx, [.mode]
	or bx, 0x4000	; enable LFB
	mov di, 0 ; not sure if some BIOSes need this... anyway it doesn't hurt
	int VIDEO_INT
	pop es

	cmp ax, 0x4f
	jne .error

	clc
	ret

.next_mode:
	mov ax, [.segment]
	mov fs, ax
	mov si, [.offset]
	jmp .find_mode

.error:
	mov si, vbe_mode_not_found
  call print
	jmp $

.width dw 0
.height dw 0
.bpp db 0
.segment dw 0
.offset dw 0
.mode dw 0

vbe_mode_not_found db "VBE mode not found!", NEWLINE, RETURN, NULL

%endif
