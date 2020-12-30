#include "kernel/hal/idt.h"
#include "kernel/hal/ports.h"
#include "kernel/hal/pic.h"
#include "kernel/drivers/display.h"
#include "kernel/panic.h"
#include "lib/string.h"

static idt_descriptor idt[IDT_MAX_ENTRIES];
static idtr idt_reg;
static ir_handler interrupt_handlers[IDT_MAX_ENTRIES];

// To print the message which defines every exception
static char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"};

void idt_install();
void idt_install_ir(int int_num, uint32_t handler);

void idt_init()
{
  idt_install_ir(0, (uint32_t)isr0);
  idt_install_ir(1, (uint32_t)isr1);
  idt_install_ir(2, (uint32_t)isr2);
  idt_install_ir(3, (uint32_t)isr3);
  idt_install_ir(4, (uint32_t)isr4);
  idt_install_ir(5, (uint32_t)isr5);
  idt_install_ir(6, (uint32_t)isr6);
  idt_install_ir(7, (uint32_t)isr7);
  idt_install_ir(8, (uint32_t)isr8);
  idt_install_ir(9, (uint32_t)isr9);
  idt_install_ir(10, (uint32_t)isr10);
  idt_install_ir(11, (uint32_t)isr11);
  idt_install_ir(12, (uint32_t)isr12);
  idt_install_ir(13, (uint32_t)isr13);
  idt_install_ir(14, (uint32_t)isr14);
  idt_install_ir(15, (uint32_t)isr15);
  idt_install_ir(16, (uint32_t)isr16);
  idt_install_ir(17, (uint32_t)isr17);
  idt_install_ir(18, (uint32_t)isr18);
  idt_install_ir(19, (uint32_t)isr19);
  idt_install_ir(20, (uint32_t)isr20);
  idt_install_ir(21, (uint32_t)isr21);
  idt_install_ir(22, (uint32_t)isr22);
  idt_install_ir(23, (uint32_t)isr23);
  idt_install_ir(24, (uint32_t)isr24);
  idt_install_ir(25, (uint32_t)isr25);
  idt_install_ir(26, (uint32_t)isr26);
  idt_install_ir(27, (uint32_t)isr27);
  idt_install_ir(28, (uint32_t)isr28);
  idt_install_ir(29, (uint32_t)isr29);
  idt_install_ir(30, (uint32_t)isr30);
  idt_install_ir(31, (uint32_t)isr31);

  PIC_remap(32, 40);

  // Install the IRQs
  idt_install_ir(32, (uint32_t)irq0);
  idt_install_ir(33, (uint32_t)irq1);
  idt_install_ir(34, (uint32_t)irq2);
  idt_install_ir(35, (uint32_t)irq3);
  idt_install_ir(36, (uint32_t)irq4);
  idt_install_ir(37, (uint32_t)irq5);
  idt_install_ir(38, (uint32_t)irq6);
  idt_install_ir(39, (uint32_t)irq7);
  idt_install_ir(40, (uint32_t)irq8);
  idt_install_ir(41, (uint32_t)irq9);
  idt_install_ir(42, (uint32_t)irq10);
  idt_install_ir(43, (uint32_t)irq11);
  idt_install_ir(44, (uint32_t)irq12);
  idt_install_ir(45, (uint32_t)irq13);
  idt_install_ir(46, (uint32_t)irq14);
  idt_install_ir(47, (uint32_t)irq15);

  idt_install();

  kprint("Interrupt Routines installed\n");
}

void irq_init()
{
  // Enable interrupts
  asm volatile("sti");

  kprint("Interrupts enabled\n");
}

void idt_install_ir_handler(uint8_t interrupt_num, ir_handler handler)
{
  interrupt_handlers[interrupt_num] = handler;
}

void idt_install()
{
  idt_reg.base = (uint32_t)&idt;
  idt_reg.limit = IDT_MAX_ENTRIES * sizeof(idt_descriptor) - 1;

  asm volatile("lidtl (%0)"
               :
               : "r"(&idt_reg));
}

void idt_install_ir(int num, uint32_t handler)
{
  if (num > IDT_MAX_ENTRIES)
  {
    return;
  }

  if (!handler)
  {
    return;
  }

  idt[num].base_lo = handler & 0xffff;
  idt[num].sel = KERNEL_CS;
  idt[num].reserved = 0;
  idt[num].flags = 0x8E;
  idt[num].base_hi = (handler >> 16) & 0xffff;
}

void isr_handler(ir_params *regs)
{
  if (interrupt_handlers[regs->int_num] != 0)
  {
    ir_handler handler = interrupt_handlers[regs->int_num];
    handler(regs);
  }
  // default handler
  else
  {
    kprintf("int num: %da\n", regs->int_num);
    kernel_panic("\n%s\n", exception_messages[regs->int_num]);
  }
}

void irq_handler(ir_params *regs)
{
  // After every interrupt we need to send an EOI to the PICs
  // or they will not send another interrupt again
  PIC_sendEOI(regs->int_num);

  if (interrupt_handlers[regs->int_num] != 0)
  {
    ir_handler handler = interrupt_handlers[regs->int_num];
    handler(regs);
  }
}
