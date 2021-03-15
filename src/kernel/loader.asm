%include "src/boot/stage2/multiboot_info.inc"
%include "src/boot/lib/vesa_structs.inc"

global _loader
extern kmain

STACKSIZE equ 0x4000
PAGE_PRESENT equ 1 << 0
PAGE_RW equ 1 << 1
PAGE_PS equ 1 << 7

; setting up the Multiboot header - see: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
MEMINFO equ 1 << 1 ; provide memory map
MULTIBOOT_VIDEO_MODE equ 1 << 2 ; align loaded modules on page boundaries

FLAGS equ MEMINFO | MULTIBOOT_VIDEO_MODE ; this is the Multiboot 'flag' field
MAGIC equ 0x1BADB002 ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS) ; checksum required
GRAPHICS_MODE equ 0
GRAPHICS_RES_X equ 1024
GRAPHICS_RES_Y equ 768
GRAPHICS_BPP equ 32

section .text
align 4
multiboot_header:
  dd MAGIC ; magic required
  dd FLAGS ; flags required
  dd CHECKSUM ; checksum required
  dd 0 ; header_addr if flags[16] is set
  dd 0 ; load_addr if flags[16] is set
  dd 0 ; load_end_addr if flags[16] is set
  dd 0 ; bss_end_addr	if flags[16] is set
  dd 0 ; entry_addr	if flags[16] is set
  dd GRAPHICS_MODE ; mode_type
  dd GRAPHICS_RES_X ; x resolution
  dd GRAPHICS_RES_Y ; y resolution
  dd GRAPHICS_BPP ; resolution bpp

; This is the virtual base address of kernel space. It must be used to convert virtual
; addresses into physical addresses until paging is enabled. Note that this is not
; the virtual address where the kernel image itself is loaded -- just the amount that must
; be subtracted from a virtual address to get a physical address.
KERNEL_VIRTUAL_BASE equ 0xc0000000 ; 3GB
FRAMEBUFFER_VIRTUAL_BASE equ 0xff400000

section .data
align 0x1000
boot_page_directory:
  ; this page directory entry identity-maps the first 4MB of the 32-bit physical address space
  dd PAGE_PS | PAGE_RW | PAGE_PRESENT
  times (1024 - 1) dd 0

; setting up entry point for linker
loader equ (_loader - KERNEL_VIRTUAL_BASE)
global loader

section .text
align 4
_loader:
  pusha

  ; EBX = framebuffer address
  mov ebx, [ebx + multiboot_info.vbe_mode_info]
  mov ebx, [ebx + vbe_screen_struct.framebuffer]

  ; map framebuffer memory
  mov eax, FRAMEBUFFER_VIRTUAL_BASE
  shr eax, 22 ; get frame index
  mov ecx, 4
  mul ecx ; multiply by 4 to get offset in bytes from page directory start
  or ebx, PAGE_PS | PAGE_RW | PAGE_PRESENT ; set page attributes
  mov dword [boot_page_directory - KERNEL_VIRTUAL_BASE + eax], ebx ; map framebuffer's first page
  add ebx, 0x400000 ; move to second frame
  mov dword [boot_page_directory - KERNEL_VIRTUAL_BASE + eax + 4], ebx ; map framebuffer's second page

  ; map kernel memory
  mov eax, KERNEL_VIRTUAL_BASE
  shr eax, 22
  mov ecx, 4
  mul ecx
  mov dword [boot_page_directory - KERNEL_VIRTUAL_BASE + eax], PAGE_PS | PAGE_RW | PAGE_PRESENT

  popa

  ; NOTE: Until paging is set up, the code must be position-independent and use physical addresses, not virtual ones!
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
  ; we need to do a long jump to the correct virtual address of start_in_higher_half which is approximately 0xc0100000.
  lea edx, [start_in_higher_half]
  jmp edx

start_in_higher_half:
  ; NOTE: From now on, paging should be enabled. The first 4MB of physical address space is
  ; mapped starting at KERNEL_VIRTUAL_BASE. Everything is linked to this address, so no more
  ; position-independent code or funny business with virtual-to-physical address translation
  ; should be necessary. We now have a higher-half kernel.
  mov esp, stack + STACKSIZE ; set up the stack

  push ebx ; multibootinfo structure pointer
  push ecx ; multibootinfo magic number

  call kmain ; call kernel
  hlt

section .bss
align 32
stack:
  resb STACKSIZE ; reserve 16k stack
