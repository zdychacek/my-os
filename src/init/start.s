[BITS 32]

section .mb_header
; Multiboot header values.

; multiboot flag bitmasks
MB_FLAGS_ALIGN equ 0x1
MB_FLAGS_MMAP  equ 0x2
MB_FLAGS_VIDEO equ 0x4
MB_FLAGS_AOUT  equ 0x8000

; multiboot constants
MB_ALIGN          equ 8
MB_MAGIC          equ 0xE85250D6
MB_ARCH_I386      equ 0
MB_TAG_TERMINATOR equ 0
MB_TAG_INFO       equ 1
MB_TAG_ALIGN      equ 6
MB_INFO_MODULES   equ 3
MB_INFO_MMAP      equ 6

; Actual multiboot header.
multiboot_header_start:
  ; magic number
  dd MB_MAGIC
  ; architecture
  dd MB_ARCH_I386
  ; header length
  dd multiboot_header_end - multiboot_header_start
  ; checksum
  dd (0 - MB_MAGIC - MB_ARCH_I386 - (multiboot_header_end - multiboot_header_start)) & 0xFFFFFFFF

  mb_tag_info_start:
    dw MB_TAG_INFO
    dw 0
    dd (mb_tag_info_end - mb_tag_info_start)
    dd MB_INFO_MODULES
    dd MB_INFO_MMAP
  mb_tag_info_end:

  mb_tag_align_start:
    dw MB_TAG_ALIGN
    dw 0
    dd (mb_tag_align_end - mb_tag_align_start)
  mb_tag_align_end:

  mb_tag_terminator_start:
    dw MB_TAG_TERMINATOR
    dw 0
    dd (mb_tag_terminator_end - mb_tag_terminator_start)
  mb_tag_terminator_end:
multiboot_header_end:

section .mb_text
global bootstrap
; Bootstrap function: called from Multiboot-compatible bootloader.
bootstrap:
  ; Disable interrupts.
  cli

  ; TODO:
  mov edi, eax
  mov esi, ebx

  ; Set up the stack.
  mov	esp, bootstrap_stack
  mov	ebp, esp

  ; Set up paging.
  call	setup_paging

  ; Set up long mode . . .
  call	setup_lmode

  ; Now, jump to 64-bit code!
  jmp	0x08:realm64

  hlt
  jmp	$

align 8
GDT:				; Global Descriptor Table (64-bit).
  .null: equ $ - GDT	; The null descriptor.
  dw 0			; Limit (low).
  dw 0			; Base (low).
  db 0			; Base (middle)
  db 0			; Access.
  db 0			; Granularity.
  db 0			; Base (high).
  .code: equ $ - GDT	; The code descriptor.
  dw 0			; Limit (low).
  dw 0			; Base (low).
  db 0			; Base (middle)
  db 10011000b		; Access.
  db 00100000b		; Granularity.
  db 0			; Base (high).
  .data: equ $ - GDT	; The data descriptor.
  dw 0			; Limit (low).
  dw 0			; Base (low).
  db 0			; Base (middle)
  db 10010010b		; Access.
  db 00000000b		; Granularity.
  db 0			; Base (high).
  .pointer:		; The GDT-pointer.
  dw $ - GDT - 1		; Limit.
  dq GDT			; Base.


[BITS 64]

extern KernelMain

; Used to call KernelMain(). Does nothing else but provide a wrapper to call high-memory code from low-memory code.
realm64:
  ; Set up segment registers.
  mov	ax, 0x10
  mov	ss, ax
  mov	ds, ax
  mov	es, ax
  mov	fs, ax
  mov	gs, ax

  ; Call KernelMain().
  ; First, set up the arguments.
  ; Calling convention uses rdi as the first argument.

  ;TODO: pass multiboot info in rdi register
  mov	r9, KernelMain
  jmp	r9
  ret

[BITS 32]

; Initializes PAE paging, maps the kernel into memory, etc.
; Returns: nothing.
setup_paging:
  ; Housekeeping: zero out all paging structures.
  mov	ecx, paging_data_end
  sub	ecx, paging_data_start
  ; Size is a multiple of 4096, so can zero out by dwords w/o problems.
  shr	ecx, 2
  xor	eax, eax
  mov	edi, paging_data_start
  rep	stosd

  ; Set up the ID mapping for the first 2MB using a 2MB page:
  ;	0x00 is the base address, e.g. the first 2MB . . .
  ;	0x03 states that the page is writable and present.
  ;	0x80 states that this is a 2MB page.
  mov	dword [paging_id_pd + 0], (0x00 | 0x03 | 0x80)
  ; Set up the loader 2MB page entry: (assumes that this assembly file
  ;	does not exceed 2MB after compilation, which is reasonable.)
  ;
  ;	0x1000000 is the load address of the kernel;
  ;	0x03 states that the page is writable and present.
  ;	0x80 states that this is a 2MB page.
  mov	dword [paging_id_pd + 8 * 8], (0x1000000 | 0x03 | 0x80)
  ; Add the PD to the PDPT.
  mov	eax, paging_id_pd
  or	eax, 3
  mov	dword [paging_id_pdpt], eax
  ; PDPT into PML4...
  mov	eax, paging_id_pdpt
  or	eax, 3
  mov	dword [paging_pml4], eax

  ; Now, time to make the kernel mappings.
  ; We have two things that we need to load: the code, and the data; code
  ; is mapped read-only and data mapped no-execute.
  ; Grab the symbols so we know where to start/end mapping.
extern kernel_pbase
extern _code_phy_begin
extern _code_phy_end
extern _data_phy_begin
extern _data_phy_end
  ; We're going to use 2MB pages for this. So the flags are 0x81 (present
  ; and 2MB page), in addition to bit 63 being set (no-execute).

  ; Since 2MB pages have to have a 2MB-aligned target, things get
  ; slightly messy here.

  ; Instead of using the physical address of the code section starting
  ; point, we need to use the kernel pbase to also map in the bootstrap
  ; code (this).
  mov	esi, kernel_pbase
  mov	edi, paging_high_pd

  cld
  ; map until we've mapped enough.
.code_map:
  ; store physical address | 0x81.
  mov	eax, esi
  or	eax, 0x81
  stosd
  ; advance cursor over second dword
  mov	eax, 0
  stosd
  ; increment cursor by 2MB
  add	esi, 0x200000
  ; are we done?
  cmp 	esi, _code_phy_end
  jl	.code_map

  ; Code's mapped in. Time for data. As before, using 2MB pages; however,
  ; the flags are a little different because we want this to be writable.
  ; So the flags are 0x83 (present, writable, and 2MB page) and there's
  ; no no-execute flag set.
  mov	esi, _data_phy_begin
  ; Assume (dangerous!) that data starts on the next 2MB page after code.
  ; So, no need to reset edi.

  ; map until we've mapped enough.
.data_map:
  ; store physical address | 0x83.
  mov	eax, esi
  or	eax, 0x83
  stosd
  ; store no-exec bit
  mov	eax, 0 ;(1<<31) ; no-execute
  stosd
  ; increment cursor by 2MB
  add	esi, 0x200000
  ; are we done?
  cmp 	esi, _data_phy_end
  jl	.data_map

  ; Okay, page directory set up. Now it's time to add the page directory
  ; into the PDPT, and the PDPT into the PML4.

  ; PD to the PDPT.
  mov	eax, paging_high_pd
  or	eax, 3
  mov	dword [paging_high_pdpt + 510 * 8], eax
  ; PDPT to PML4...
  mov	eax, paging_high_pdpt
  or	eax, 3
  mov	dword [paging_pml4 + 511 * 8], eax

  ; Set up physical memory mapping.
  ; Set up the PD.
  mov	eax, 0x83
  ; Map the four GB via four PDs.
  mov	ecx, 2048
  mov	edi, paging_phy_pd
.phy_pdpt_loop:
  mov	dword [edi], eax
  mov	dword [edi + 4], 0
  add	edi, 8
  add	eax, 0x200000
  loop	.phy_pdpt_loop

  ; Put the PDs into the PDPT.
  mov	eax, paging_phy_pd
  or	eax, 3
  mov	dword [paging_phy_pdpt], eax
  add	eax, 0x1000
  mov	dword [paging_phy_pdpt + 8], eax
  add	eax, 0x1000
  mov	dword [paging_phy_pdpt + 16], eax
  add	eax, 0x1000
  mov	dword [paging_phy_pdpt + 24], eax

  ; Point the correct entry in the PML4 to the physical memory map.
  mov	eax, paging_phy_pdpt
  or	eax, 3
  mov	dword [paging_pml4 + 3072], eax

  ; Finally, fill in cr3 with the address of the PML4.
  mov	eax, paging_pml4
  mov	cr3, eax

  ret

; Sets up long mode, brings CPU into compatibility mode.
setup_lmode:
  ; Set PAE paging bit.
  mov	eax, cr4
  or	eax, 1 << 5
  mov	cr4, eax

  ; Set PGE (global pages) bit.
  mov eax, cr4
  or eax, 1 << 7
  mov cr4, eax

  ; Set bit 8of the EFER MSR to enable long mode, and bit 11 for NXE.
  mov	ecx, 0xc0000080
  rdmsr
  or	eax, 1 << 8
  or	eax, 1 << 11
  wrmsr

  ; Actually enable paging . . .
  mov	eax, cr0
  or	eax, 1 << 31
  ; Enable write-protection (CR0.WP) while we're at it.
  or 	eax, 1 << 16
  mov	cr0, eax

  ; Load the new (provisional, will be replaced later) GDT.
  lgdt	[GDT.pointer]

  ; All that's left is to actually jump into 64-bit code, which will be done elsewhere.
  ret

; Prints a message to the next unused line on screen.
; Requires: eax to be set to the line number, esi to be set to the input string.
; Returns: nothing.
debug_message:
  pushad
  mov	edi, 0xb8000

  movzx	ebx, byte [bootstrap_last_line]

  inc	byte [bootstrap_last_line]

  mov	ebx, 160		; 160: number of bytes per 80-character line.
  mul	ebx
  add	edi, eax
  xor	eax, eax
.write_loop:
  mov	al, byte [esi]
  mov	byte [edi], al
  inc	esi
  inc	edi
  mov	byte [edi], 0x7
  inc	edi
  mov	al, byte [esi]
  test	al, al
  jnz	.write_loop

  popad
  ret

; Prints a value to the next unused line on the screen.
; Requires: eax to be set to the value to print.
; Returns: nothing.
debug_value:
  mov	edi, 0xb8000

  movzx	ebx, byte [bootstrap_last_line]
  inc	byte [bootstrap_last_line]

  mov	esi, eax		; Move the value into esi.

  mov	eax, 160		; 160: number of bytes per 80-character line.
  mul	ebx
  add	edi, eax

  mov	ecx, 28			; Value to shift by.
.digit_loop:
  mov	ebx, esi		; Copy the value to print.
  shr	ebx, cl			; Shift right.
  and	ebx, 0xf		; Bitwise and to mask out the other bits.
  ; Look up the required character.
  mov	eax, hex_table
  add	eax, ebx
  mov	al, byte [eax]
  ; Character now in al; use it.
  mov	byte [edi], al
  mov	byte [edi + 1], 7
  add	edi, 2

  ; Check the value of ecx.
  mov	eax, ecx
  test	eax, eax
  jz	.digit_loop_finished
  sub	ecx, 4
  jmp	.digit_loop
.digit_loop_finished:
  ret

error:
  mov	esi, error_string
  mov	eax, 0
  call	debug_message
  hlt
  jmp	$

section .mb_rodata
loading_string:		db	"Loading . . ."
      db	0
error_string:		db	"Error."
      db	0
hex_table:		db	"0123456789ABCDEF"

section .mb_bss nobits
  resb	4096			; Reserve a 4KB stack.
bootstrap_stack:			; Bootstrap stack top.
; Paging structures.
align	4096
paging_data_start:
paging_pml4:
  resq	512
paging_id_pdpt:
  resq	512
paging_id_pd:
  resq	512
paging_high_pdpt:
  resq	512
paging_high_pd:
  resq	512
paging_phy_pdpt:
  resq	512
paging_phy_pd: ; maps the first four GB
  resq	2048
paging_data_end:

; Misc. variables.
bootstrap_last_line:
  resd	1
