bits 16

%define MEMORY_MAP_ENTRY_LEN 24

;---------------------------------------------
; Bios function INT 15h, AX=E820h
; EBX must contain 0 on first call, and remain unchanged
; on subsequent calls until it is zero again.
;	ret\ ax=entries buffer length in bytes
; ret\ [es:di]=memory map location to be placed at 
;---------------------------------------------
get_memory_map:
  xor bp, bp
  xor ebx, ebx ; ebx must be 0 to start
  mov edx, 'PAMS' ; Place "SMAP" into edx
  mov eax, 0xe820
  mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
  mov ecx, MEMORY_MAP_ENTRY_LEN	; ask for 24 bytes
  int MISC_INT
  jc .fail ; carry set on first call means "unsupported function"

  mov edx, 'PAMS'
  cmp eax, edx ; on success, eax must have been set to "SMAP"
  jne .fail

  test ebx, ebx ; ebx = 0 implies list is only 1 entry long (worthless)
  je .fail

  jmp .loop

.e820lp:
  mov eax, 0xe820	; eax, ecx get trashed on every int 0x15 call
  mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
  mov ecx, MEMORY_MAP_ENTRY_LEN ; ask for 24 bytes again
  int MISC_INT
  jc .done ; carry set means "end of list already reached"
  mov edx, 'PAMS' ; repair potentially trashed register
.loop:
  jcxz .skip ; skip any 0 length entries
  cmp cl, 20 ; got a 24 byte ACPI 3.X response?
  jbe .notext
  test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
  je .skip
.notext:
  mov ecx, [es:di + 8] ; get lower uint32_t of memory region length
  or ecx, [es:di + 12] ; "or" it with upper uint32_t to test for zero
  jz .skip ; if length uint64_t is 0, skip entry
  inc bp
  add di, MEMORY_MAP_ENTRY_LEN
.skip:
  test ebx, ebx	; if ebx resets to 0, list is complete
  jne .e820lp
.done:
  clc	; there is "jc" on end of list to this point, so the carry must be cleared
  ; Count entries buffer bytes length 
  mov ax, bp
  mov bx, MEMORY_MAP_ENTRY_LEN
  mul bx
  
  ret
.fail:
  stc	; "function unsupported" error exit
  ret

;---------------------------------------------
;	Get memory size for >64M configuations
;	ret\ ax=KB between 1MB and 16MB
;	ret\ bx=number of 64K blocks above 16MB
;	ret\ bx=0 and ax= -1 on error
;---------------------------------------------
get_memory_size:
	push	ecx
	push	edx
	xor		ecx, ecx
	xor		edx, edx
	mov		ax, 0xe801
	int		MISC_INT	
	jc		.error
	cmp		ah, 0x86		;unsupported function
	je		.error
	cmp		ah, 0x80		;invalid command
	je		.error
	jcxz	.use_ax			;bios may have stored it in ax,bx or cx,dx. test if cx is 0
	mov		ax, cx			;its not, so it should contain mem size; store it
	mov		bx, dx

.use_ax:
	pop		edx				;mem size is in ax and bx already, return it
	pop		ecx
	ret

.error:
	mov		ax, -1
	mov		bx, 0
	pop		edx
	pop		ecx
	ret
