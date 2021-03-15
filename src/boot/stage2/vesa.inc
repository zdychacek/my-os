%ifndef VESA_INC
%define VESA_INC

%include "src/boot/lib/constants.inc"
%include "src/boot/lib/terminal.inc"
%include "src/boot/lib/vesa_structs.inc"

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
