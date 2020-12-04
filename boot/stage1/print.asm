print:
	lodsb
	or al, al		; test for NULL termination
	jz .printdone
	mov ah, 0eh
	int 10h
	jmp print
.printdone:
	ret
