%ifndef TERMINAL_INC
%define TERMINAL_INC

VIDEO_MEMORY equ 0xb8000
VGA_WIDTH equ 80
VGA_HEIGHT equ 25
VGA_COLOR_BLACK equ 0
VGA_COLOR_BLUE equ 1
VGA_COLOR_GREEN equ 2
VGA_COLOR_CYAN equ 3
VGA_COLOR_RED equ 4
VGA_COLOR_MAGENTA equ 5
VGA_COLOR_BROWN equ 6
VGA_COLOR_LIGHT_GREY equ 7
VGA_COLOR_DARK_GREY equ 8
VGA_COLOR_LIGHT_BLUE equ 9
VGA_COLOR_LIGHT_GREEN equ 10
VGA_COLOR_LIGHT_CYAN equ 11
VGA_COLOR_LIGHT_RED equ 12
VGA_COLOR_LIGHT_MAGENTA equ 13
VGA_COLOR_LIGHT_BROWN equ 14
VGA_COLOR_WHITE equ 15

; IN = dl: y, dh: x
; OUT = bx: Index with offset 0xB8000 at VGA buffer
; Other registers preserved
terminal_get_index:
  push ax
  push cx

  movzx bx, dl ; y
  movzx cx, dh ; x

  shl cx, 1 ; multiply by two because every entry is a word that takes up 2 bytes

  mov ax, VGA_WIDTH
  mul bx
  mov bx, ax

  shl bx, 1
  add bx, cx

  pop cx
  pop ax
  ret

; IN = dl: bg color, dh: fg color
; OUT = none
terminal_set_color:
  shl dl, 4

  or dl, dh
  mov [terminal_color], dl

  ret

; IN = dl: y, dh: x, al: ASCII char
; OUT = none
terminal_put_entry_at:
  pusha

  call terminal_get_index

  mov dl, [terminal_color]
  mov byte [VIDEO_MEMORY + ebx], al
  mov byte [VIDEO_MEMORY + 1 + ebx], dl

  popa
  ret

; IN = al: ASCII char
terminal_put_char:
  ; This loads terminal_column at DH, and terminal_row at DL
  mov dx, [terminal_cursor_pos]

  call terminal_put_entry_at

  inc dh
  cmp dh, VGA_WIDTH
  jne .cursor_moved

  mov dh, 0
  inc dl

  cmp dl, VGA_HEIGHT
  jne .cursor_moved

  mov dl, 0

  .cursor_moved:
    ; Store new cursor position
    mov [terminal_cursor_pos], dx

  ret

; IN = ESI: string location
; OUT = none
terminal_write:
  pusha

  mov edx, 0

  .loopy:
    mov al, [esi]

    ; check for NULL character
    cmp al, NULL
    je .done

    call terminal_put_char

    inc esi
    jmp .loopy

  .done:
    popa
    ret

; Writes string with new line.
; IN = ESI: string location
; OUT = none
terminal_write_line:
  pusha

  call terminal_write
  call terminal_write_nl

  popa
  ret

; Writes new line (advance to new line).
; IN = none
; OUT = none
terminal_write_nl:
  pusha

  mov dx, [terminal_cursor_pos]
  mov dh, 0 ; carriage return
  inc dl ; new line
  mov [terminal_cursor_pos], dx

  popa
  ret

; Writes error message.
; IN = ESI: string location
; OUT = none
terminal_write_error:
  pusha

  mov dl, VGA_COLOR_BLACK
  mov dh, VGA_COLOR_RED
  call terminal_set_color

  call terminal_write_line

  ; resrtore color
  mov dl, VGA_COLOR_BLACK
  mov dh, VGA_COLOR_WHITE
  call terminal_set_color

  popa
  ret

; Writes success message.
; IN = ESI: string location
; OUT = none
terminal_write_success:
  pusha

  mov dl, VGA_COLOR_BLACK
  mov dh, VGA_COLOR_GREEN
  call terminal_set_color

  call terminal_write_line

  ; resrtore color
  mov dl, VGA_COLOR_BLACK
  mov dh, VGA_COLOR_WHITE
  call terminal_set_color

  popa
  ret

; Print hex representation of a number.
; IN = edx: number
; OUT = none
terminal_write_hex:
  pusha

  mov edi, out_hextstr
  mov eax, edx
  mov esi, hexstr
  mov ecx, 8

  .hexloop:
    rol eax, 4 ; leftmost will become rightmost
    mov ebx, eax
    and ebx, 0x0f
    mov bl, [esi + ebx] ; index into hexstr
    mov [edi], bl
    inc edi
    dec ecx
    jnz .hexloop

  mov esi, out_hextstr_prefix
  call terminal_write
  mov esi, out_hextstr
  call terminal_write

  popa
  ret

; Print hex representation of a number with new line.
; IN = edx: number
; OUT = none
terminal_write_hex_line:
  pusha

  call terminal_write_hex
  call terminal_write_nl

  popa
  ret

hexstr db '0123456789ABCDEF'
out_hextstr_prefix db '0x', NULL
out_hextstr db '00000000', NULL

; white on black
terminal_color db 0x0f

terminal_cursor_pos:
  terminal_column db 0
  terminal_row db 0

%endif
