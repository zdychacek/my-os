#pragma once

typedef struct _bootconfig
{
  char *file;
} bootconfig;

bootconfig *parse_config(char *config_text);
