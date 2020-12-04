#include "vga.h"
#include "bootconfig.h"
#include "lib.h"

bootconfig *parse_config(char *config_text)
{
  bootconfig *cfg = (bootconfig *)malloc(sizeof(bootconfig));

  cfg->file = config_text;

  return cfg;
}
