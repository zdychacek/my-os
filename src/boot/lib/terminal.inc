%ifndef LIB_TERMINAL_INC
%define LIB_TERMINAL_INC

%define VIDEO_OUT 0xe ; Teletype output command

; Prints string (null terminated) to the screen.
; si = pointer to number to print
print:
  pusha

  ; keep this in mind:
  ; while (string[i] != 0) { print string[i]; i++ }

  ; the comparison for string end (null byte)
  .start:
    mov al, [si] ; 'si' is the base address for the string
    cmp al, 0
    je .done

    ; the part where we print with the BIOS help
    mov ah, VIDEO_OUT
    int VIDEO_INT ; 'al' already contains the char

    ; increment pointer and do next loop
    add si, 1
    jmp .start

  .done:
    popa
    ret

; Prints new line to the screen.
print_nl:
  mov si, NL
  call print
  ret

NL db NEWLINE, RETURN, NULL

%endif
