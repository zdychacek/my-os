#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 512
#define PART_TABLE 446

int main(int argc, char *argv[])
{

  FILE *boot, *disk;
  char *sboot, *sdisk;
  int mbr = 0;
  int diskoffset = 0;

  char bootloader[SECTOR_SIZE * 2];
  char cursector[SECTOR_SIZE];
  char newsector[SECTOR_SIZE * 2];
  size_t ret;

  if (!(argc == 3))
  {
    fprintf(stderr, "Invalid parameters\n");
    exit(-1);
  }

  printf("Installing MBR from image %s to %s\n", argv[1], argv[2]);

  sboot = argv[1];
  sdisk = argv[2];

  boot = fopen(sboot, "rb");
  disk = fopen(sdisk, "rb+");

  if (boot == NULL || disk == NULL)
  {
    fprintf(stderr, "unable to open file(s)\n");
    exit(-1);
  }

  fseek(boot, 0, SEEK_SET);
  fseek(disk, 0, SEEK_SET);

  ret = fread(bootloader, 1, SECTOR_SIZE * 2, boot);

  if (ret != SECTOR_SIZE * 2)
  {
    fprintf(stderr, "error reading %s of improper file size: %zu\n", sboot, ret);
    exit(-1);
  }

  int i;

  ret = fread(cursector, 1, SECTOR_SIZE, disk);

  if (ret != SECTOR_SIZE)
  {
    fprintf(stderr, "error reading %s or improper file size\n", sdisk);
    exit(-1);
  }

  fseek(disk, 0, SEEK_SET);

  memcpy(newsector, bootloader, SECTOR_SIZE * 2);
  memcpy(newsector + PART_TABLE, cursector + PART_TABLE, SECTOR_SIZE - PART_TABLE);

  newsector[510] = 0x55;
  newsector[511] = 0xAA;

  fwrite(newsector, 1, SECTOR_SIZE * 2, disk);

  fclose(boot);
  fclose(disk);

  return 0;
}
