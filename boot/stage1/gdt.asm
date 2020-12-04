; GLOBAL DESCRIPTOR TABLE
align 32

gdt: ; Address for the GDT

gdt_null:
	dd 0
	dd 0

gdt_kernel_code:
	dw 0xFFFF ; Limit 0xFFFF
	dw 0 ; Base 0:15
	db 0 ; Base 16:23
	db 0x9A ; Present, Ring 0, Code, Non-conforming, Readable
	db 0xCF ; Page-granular
	db 0 ; Base 24:31

gdt_kernel_data:
	dw 0xFFFF ; Limit 0xFFFF
	dw 0 ; Base 0:15
	db 0 ; Base 16:23
	db 0x92 ; Present, Ring 0, Code, Non-conforming, Readable
	db 0xCF	; Page-granular
	db 0 ; Base 24:31

gdt_end: ; Used to calculate the size of the GDT

gdt_desc: ; The GDT descriptor
	dw gdt_end - gdt - 1 ; Limit (size)
	dd gdt ; Address of the GDT
