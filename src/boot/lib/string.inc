%ifndef STRING_INC
%define STRING_INC

; Get string length.
; di = string to get length for
; ax = returns string length
strlen:
  push di
  push cx
  xor cx, cx ; clear out counter
.next:
  cmp [di], byte 0 ; null byte yet?
  jz .null ; yes, get out
  inc cx ; char is ok, count it
  inc di ; move to next char
  jmp .next ; process again
.null:
  mov ax, cx ; cx = the length (put in ax)
  pop cx ; restore cx
  pop di
  ret ; get out

%endif
