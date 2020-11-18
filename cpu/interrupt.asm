; Defined in isr.c
[extern isr_handler]
[extern irq_handler]

; We don't get information about which interrupt was called
; when the handler is run, so we will need to have a different handler
; for every interrupt.
; Furthermore, some interrupts push an error code onto the stack but others
; don't, so we will push a dummy error code for those which don't, so that
; we have a consistent stack for all of them.

%macro no_error_code_interrupt_handler 1
  global isr%1 ; First make the ISRs global
  isr%1:
    push dword 0 ; push 0 as error code
    push dword %1 ; push the interrupt number
    jmp isr_common_stub ; jump to the common handler
%endmacro

%macro error_code_interrupt_handler 1
  global isr%1 ; First make the ISRs global
  isr%1:
    push dword %1 ; push the interrupt number
    jmp isr_common_stub  ; jump to the common handler
%endmacro

%macro interrupt_request_handler 2
  global irq%1
  irq%1:
    cli
    push dword %1
    push dword %2
    jmp irq_common_stub
%endmacro

; Common ISR code
isr_common_stub:
  ; 1. Save CPU state
	pusha ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
	mov ax, ds ; Lower 16-bits of eax = ds.
	push eax ; save the data segment descriptor
	mov ax, 0x10  ; kernel data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

  ; 2. Call C handler
	call isr_handler

  ; 3. Restore state
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popa
	add esp, 8 ; Cleans up the pushed error code and pushed ISR number
	sti
	iret ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

; Common IRQ code. Identical to ISR code except for the 'call'
; and the 'pop ebx'
irq_common_stub:
  pusha
  mov ax, ds
  push eax
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  call irq_handler ; Different than the ISR code

  pop ebx  ; Different than the ISR code
  mov ds, bx
  mov es, bx
  mov fs, bx
  mov gs, bx
  popa
  add esp, 8
  sti
  iret

; 0: Divide By Zero Exception
no_error_code_interrupt_handler 0
; 1: Debug Exception
no_error_code_interrupt_handler 1
; 2: Non Maskable Interrupt Exception
no_error_code_interrupt_handler 2
; 3: Int 3 Exception
no_error_code_interrupt_handler 3
; 4: INTO Exception
no_error_code_interrupt_handler 4
; 5: Out of Bounds Exception
no_error_code_interrupt_handler 5
; 6: Invalid Opcode Exception
no_error_code_interrupt_handler 6
; 7: Coprocessor Not Available Exception
no_error_code_interrupt_handler 7
; 8: Double Fault Exception (With Error Code!)
error_code_interrupt_handler              8
; 9: Coprocessor Segment Overrun Exception
no_error_code_interrupt_handler 9
; 10: Bad TSS Exception (With Error Code!)
error_code_interrupt_handler              10
; 11: Segment Not Present Exception (With Error Code!)
error_code_interrupt_handler              11
; 12: Stack Fault Exception (With Error Code!)
error_code_interrupt_handler              12
; 13: General Protection Fault Exception (With Error Code!)
error_code_interrupt_handler              13
; 14: Page Fault Exception (With Error Code!)
error_code_interrupt_handler              14
; 15: Reserved Exception
no_error_code_interrupt_handler 15
; 16: Floating Point Exception
no_error_code_interrupt_handler 16
; 17: Alignment Check Exception
no_error_code_interrupt_handler 17
; 18: Machine Check Exception
no_error_code_interrupt_handler 18
; 19: Reserved
no_error_code_interrupt_handler 19
; 20: Reserved
no_error_code_interrupt_handler 20
; 21: Reserved
no_error_code_interrupt_handler 21
; 22: Reserved
no_error_code_interrupt_handler 22
; 23: Reserved
no_error_code_interrupt_handler 23
; 24: Reserved
no_error_code_interrupt_handler 24
; 25: Reserved
no_error_code_interrupt_handler 25
; 26: Reserved
no_error_code_interrupt_handler 26
; 27: Reserved
no_error_code_interrupt_handler 27
; 28: Reserved
no_error_code_interrupt_handler 28
; 29: Reserved
no_error_code_interrupt_handler 29
; 30: Reserved
no_error_code_interrupt_handler 30
; 31: Reserved
no_error_code_interrupt_handler 31

; IRQ handlers
interrupt_request_handler 0, 32
interrupt_request_handler 1, 33
interrupt_request_handler 2, 34
interrupt_request_handler 3, 35
interrupt_request_handler 4, 36
interrupt_request_handler 5, 37
interrupt_request_handler 6, 38
interrupt_request_handler 7, 39
interrupt_request_handler 8, 40
interrupt_request_handler 9, 41
interrupt_request_handler 10, 42
interrupt_request_handler 11, 43
interrupt_request_handler 12, 44
interrupt_request_handler 13, 45
interrupt_request_handler 14, 46
interrupt_request_handler 15, 47
