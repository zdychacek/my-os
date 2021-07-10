OUTPUT_FORMAT(elf64-x86-64)
ENTRY(bootstrap)

kernel_pbase = 0x1000000;
kernel_vbase = 0xffffffff80000000;

PHDRS {
    header PT_LOAD;
    code PT_LOAD;
    data PT_LOAD;
}

SECTIONS {
    . = kernel_pbase;

    .mb_header : {
        *(.mb_header)
    } :header

    .bootstrap : {
        *(.mb_text)
        *(.mb_rodata)
        *(.mb_bss)

        . = ALIGN(4096);
    } :header

    . += kernel_vbase - kernel_pbase;
    _start = .;

    .text : AT(ADDR(.text) - kernel_vbase + kernel_pbase) {
        _code = .;
        _code_phy_begin = . + kernel_pbase - kernel_vbase;
        *(EXCLUDE_FILE(*start.o) .text)
        *(EXCLUDE_FILE(*start.o) .rodata*)

        _eh_frame = .;
        *(.eh_frame)
        _code_end = .;
        _code_phy_end = . + kernel_pbase - kernel_vbase;
    } :code

    . = ALIGN(0x200000);

    .data : AT(ADDR(.data) - kernel_vbase + kernel_pbase) {
        _data = .;
        _data_phy_begin = . + kernel_pbase - kernel_vbase;
        *(.data)
    } :data

    .bss : AT(ADDR(.bss) - kernel_vbase + kernel_pbase) {
        _bss_start = .;
        *(EXCLUDE_FILE(*start.o) .bss)

        *(COMMON)
        _bss_end = .;
        _data_end = .;
        _data_phy_end = . + kernel_pbase - kernel_vbase;
    } :data

    _end = .;

    /DISCARD/ : {
        *(.comment)
    }
}
