STACKSIZE equ 0x4000

global _loader
extern kmain

; This is the virtual base address of kernel space. It must be used to convert virtual
; addresses into physical addresses until paging is enabled. Note that this is not
; the virtual address where the kernel image itself is loaded -- just the amount that must
; be subtracted from a virtual address to get a physical address.
KERNEL_VIRTUAL_BASE equ 0xC0000000 ; 3GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22) ; Page directory index of kernel's 4MB PTE.

section .data
align 0x1000
boot_page_directory:
  ; This page directory entry identity-maps the first 4MB of the 32-bit physical address space.
  ; All bits are clear except the following:
  ; bit 7: PS The kernel page is 4MB.
  ; bit 1: RW The kernel page is read/write.
  ; bit 0: P  The kernel page is present.
  ; This entry must be here -- otherwise the kernel will crash immediately after paging is
  ; enabled because it can't fetch the next instruction! It's ok to unmap this page later.
  dd 0x00000083
  times (KERNEL_PAGE_NUMBER - 1) dd 0 ; Pages before kernel space.
  ; This page directory entry defines a 4MB page containing the kernel.
  dd 0x00000083
  times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0 ; Pages after the kernel image.

; setting up entry point for linker
loader equ (_loader - 0xC0000000)
global loader

_loader:
  ; NOTE: Until paging is set up, the code must be position-independent and use physical
  ; addresses, not virtual ones!
  mov edx, (boot_page_directory - KERNEL_VIRTUAL_BASE)
  mov cr3, edx ; Load Page Directory Base Register.

  mov edx, cr4
  or edx, 0x00000010 ; Set PSE bit in CR4 to enable 4MB pages.
  mov cr4, edx

  mov edx, cr0
  or edx, 0x80000000 ; Set PG bit in CR0 to enable paging.
  mov cr0, edx

  ; Start fetching instructions in kernel space.
  ; Since eip at this point holds the physical address of this command (approximately 0x00100000)
  ; we need to do a long jump to the correct virtual address of start_in_higher_half which is
  ; approximately 0xC0100000.
  lea edx, [start_in_higher_half]
  jmp edx                                                     ; NOTE: Must be absolute jump!

start_in_higher_half:
  ; NOTE: From now on, paging should be enabled. The first 4MB of physical address space is
  ; mapped starting at KERNEL_VIRTUAL_BASE. Everything is linked to this address, so no more
  ; position-independent code or funny business with virtual-to-physical address translation
  ; should be necessary. We now have a higher-half kernel.
  mov esp, stack + STACKSIZE ; set up the stack

  ; pass Multiboot info structure -- WARNING: This is a physical address and may not be
  ; in the first 4MB!
  push ebx ; multibootinfo structure pointer
  push ecx ; multibootinfo magic number

  call kmain ; call kernel
  hlt

section .bss
align 32
stack:
  resb STACKSIZE ; reserve 16k stack
