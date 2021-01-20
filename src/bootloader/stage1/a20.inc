%ifndef A20_inc
%define A20_inc

%define A20_GATE 0x92 ; Fast A20 gate
%define A20_ENABLED 0b10 ; Bit 1 defines whether A20 is enabled

enable_a20:
  in al, A20_GATE
  or al, A20_ENABLED
  out A20_GATE, al
  ret

%endif
