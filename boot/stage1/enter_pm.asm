enter_pm:
	cli ; Turn off interrupts
	in al, 0x92	; Enable a20
	or al, 2
	out 0x92, al

	xor ax, ax ; Clear AX register
	mov ds, ax ; Set DS-register to 0
	mov es, ax
	mov fs, ax
	mov gs, ax

	lgdt [gdt_desc] ; Load the Global Descriptor Table

  ; Enter Protected Mode
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp 08h:pm ; Jump to code segment, offset kernel_segments

[BITS 32]

pm:
	pop ecx	; End of memory map
	mov [mem_info+4], ecx
	pop ecx
	mov [mem_info], ecx

	xor eax, eax

	mov ax, 0x10 ; Save data segment identifier
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	mov esp, 0x00900000	; Move temp stack pointer to 090000h

	mov eax, mem_info
	push eax
	mov edx, 0x00050000
	lea eax, [edx]
	call eax ; stage2_main(mem_info)
