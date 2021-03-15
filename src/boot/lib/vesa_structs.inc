%ifndef VESA_STRUCTS_INC
%define VESA_STRUCTS_INC

struc vbe_info_block_struct
  .signature resb	4
  .version resw 1
  .oem resd 1
  .capabilities resd 1
  .video_modes resd 1
  .video_memory resw 1
  .software_rev resw 1
  .vendor resd 1
  .product_name resd 1
  .product_rev resd 1
  .reserved resb 222
  .oem_data resb 256
endstruc

struc vbe_mode_info_block_struct
  .attributes resw 1 ; deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
  .window_a resb 1 ; deprecated
  .window_b resb 1  ; deprecated
  .granularity resw 1 ; deprecated ; used while calculating bank numbers
  .window_size resw 1
  .segment_a resw 1
  .segment_b resw 1
  .win_func_ptr resd 1 ; deprecated ; used to switch banks from protected mode without returning to real mode
  .pitch resw 1 ;	number of bytes per horizontal line
  .width resw 1 ; width in pixels
  .height resw 1 ; height in pixels
  .w_char resb 1 ; unused...
  .y_char resb 1 ; unused...
  .planes resb 1
  .bpp resb 1 ;	bits per pixel in this mode
  .banks resb 1 ; deprecated ; total number of banks in this mode
  .memory_model resb 1
  .bank_size resb 1 ; deprecated ; size of a bank, almost always 64 KB but may be 16 KB...
  .image_pages resb 1
  .reserved0 resb 1

  .red_mask resb 1
  .red_position resb 1
  .green_mask resb 1
  .green_position resb 1
  .blue_mask resb 1
  .blue_position resb 1
  .reserved_mask resb 1
  .reserved_position resb 1
  .direct_color_attributes resb 1

  .framebuffer resd 1 ; physical address of the linear frame buffer ; write here to draw to the screen
  .off_screen_mem_off resd 1
  .off_screen_mem_size resw 1 ;	size of memory in the framebuffer but not being displayed on the screen
  .reserved1 resb 206
endstruc

struc vbe_screen_struct
  .width resw	1
  .height resw 1
  .framebuffer resd 1
  .pitch resw 1
  .bpp resb 1
  .bytes_per_pixel resw 1
endstruc

%endif
