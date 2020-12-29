[bits 32]

global _load_page_directory
global _get_page_directory
global _enable_paging
global _is_paging_enabled
global _flush_tlb_entry

_load_page_directory:
  push ebp
  mov ebp, esp
  mov eax, [ebp + 8]

  mov cr3, eax
  mov esp, ebp

  pop ebp
  ret

_get_page_directory:
  mov eax, cr3
  ret

_enable_paging:
  push ebp
  mov ebp, esp
  mov eax, [ebp + 8]

  mov ebx, cr0
  cmp	eax, 1
  je .enable
  jmp	.disable
  .enable:
    or ebx, 0x80000000
    mov cr0, ebx
    jmp	.done
  .disable:
    and ebx, 0x7FFFFFFF
    mov cr0, ebx
  .done:
    pop ebp
    ret

_is_paging_enabled:
  mov eax, cr0
  and eax, 0x80000000
  jnz .enabled
  .disabled:
    mov eax, 0
    ret
  .enabled:
    mov eax, 1
    ret

_flush_tlb_entry:
  push ebp
  mov ebp, esp
  mov eax, [ebp + 8]

  cli
  invlpg [eax]
  sti

  pop ebp
  ret
