#pragma once

#include "lib/types.h"

// Segment selector
#define KERNEL_CS 0x08
#define IDT_MAX_ENTRIES 256

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

// ISRs reserved for CPU exceptions
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// IRQs
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

typedef struct
{
  uint32_t ds;                                         // Data segment selector
  uint32_t edi, esi, ebp, useless, ebx, edx, ecx, eax; // Pushed by pusha.
  uint32_t int_num, err_code;                          // Interrupt number and error code (if applicable)
  uint32_t eip, cs, eflags, esp, ss;                   // Pushed by the processor automatically
} __attribute__((packed)) ir_params;

typedef struct
{
  uint16_t base_lo; // Lower 16 bits of handler function address
  uint16_t sel;     // Kernel segment selector
  uint8_t reserved;
  /* First byte
     * Bit 7: "Interrupt is present"
     * Bits 6-5: Privilege level of caller (0=kernel..3=user)
     * Bit 4: Set to 0 for interrupt gates
     * Bits 3-0: bits 1110 = decimal 14 = "32 bit interrupt gate" */
  uint8_t flags;
  uint16_t base_hi; // Higher 16 bits of handler function address
} __attribute__((packed)) idt_descriptor;

typedef struct
{
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idtr;

typedef void (*ir_handler)(ir_params *);

void idt_init();
void irq_init();
void idt_install_ir_handler(uint8_t int_num, ir_handler handler);
