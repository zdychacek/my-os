#include "bootloader/stage2/bootconfig.h"
#include "bootloader/stage2/lib.h"

bootconfig *parse_config(char *config_text)
{
  bootconfig *cfg = (bootconfig *)malloc(sizeof(bootconfig));

  cfg->file = config_text;

  return cfg;
}
