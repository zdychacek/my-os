#include "kernel/drivers/keyboard.h"
#include "kernel/hal/ports.h"
#include "kernel/hal/idt.h"
#include "kernel/drivers/display.h"
#include "kernel/main.h"
#include "lib/string.h"

#define BACKSPACE 0x0E
#define ENTER 0x1C

static char key_buffer[256];

#define SC_MAX 57
const char *sc_name[] = {"ERROR", "Esc", "1", "2", "3", "4", "5", "6",
                         "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E",
                         "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl",
                         "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
                         "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".",
                         "/", "RShift", "Keypad *", "LAlt", "Spacebar"};
const char sc_ascii[] = {'?', '?', '1', '2', '3', '4', '5', '6',
                         '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
                         'U', 'I', 'O', 'P', '[', ']', '?', '?', 'A', 'S', 'D', 'F', 'G',
                         'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V',
                         'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

static void keyboard_callback(ir_params *regs)
{
  /* The PIC leaves us the scancode in port 0x60 */
  uint8_t scancode = port_byte_read(0x60);

  if (scancode > SC_MAX)
    return;
  if (scancode == BACKSPACE)
  {
    backspace(key_buffer);
    kprint_backspace();
  }
  else if (scancode == ENTER)
  {
    kprint("\n");
    user_input(key_buffer); /* kernel-controlled function */
    key_buffer[0] = '\0';
  }
  else
  {
    char letter = sc_ascii[(int)scancode];
    /* Remember that kprint only accepts char[] */
    char str[2] = {letter, '\0'};
    append(key_buffer, letter);
    kprint(str);
  }
  unused(regs);
}

void keyboard_init()
{
  idt_install_ir_handler(IRQ1, keyboard_callback);

  kprint("Keyboard initialized\n");
}
