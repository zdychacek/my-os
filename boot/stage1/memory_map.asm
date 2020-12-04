; Bios function INT 15h, AX=E820h
; EBX must contain 0 on first call, and remain unchanged
; 	on subsequent calls until it is zero again
; Code adapted from OSDEV wiki
; Memory map is 24 byte struct placed at [ES:DI]
memory_map:
	xor ebx, ebx				; ebx must be 0 to start
	mov edx, 0x0534D4150		; Place "SMAP" into edx
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24					; ask for 24 bytes
	int 0x15
	jc short .fail				; carry set on first call means "unsupported function"

	mov edx, 0x0534D4150		;
	cmp eax, edx				; on success, eax must have been set to "SMAP"
	jne short .fail

	test ebx, ebx				; ebx = 0 implies list is only 1 entry long (worthless)
	je short .fail

	jmp short .loop

.e820lp:
	mov eax, 0xe820				; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24					; ask for 24 bytes again
	int 0x15
	jc short .done				; carry set means "end of list already reached"
	mov edx, 0x0534D4150		; repair potentially trashed register
.loop:
	jcxz .skip					; skip any 0 length entries
	cmp cl, 20					; got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je short .skip
.notext:
	mov ecx, [es:di + 8]		; get lower uint32_t of memory region length
	or ecx, [es:di + 12]		; "or" it with upper uint32_t to test for zero
	jz .skip					; if length uint64_t is 0, skip entry
	add di, 24

.skip:
	test ebx, ebx				; if ebx resets to 0, list is complete
	jne short .e820lp
.done:
	clc							; there is "jc" on end of list to this point, so the carry must be cleared
	ret
.fail:
	stc							; "function unsupported" error exit
	ret

align 16

mem_info:
	dd 0
	dd 0
