#pragma once

typedef struct
{
  char *file;
} bootconfig;

bootconfig *parse_config(char *config_text);
