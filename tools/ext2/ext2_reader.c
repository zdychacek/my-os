#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct superblock_
{
  uint32_t inodes_count;   // Total # of inodes
  uint32_t blocks_count;   // Total # of blocks
  uint32_t r_blocks_count; // # of reserved blocks for superuser
  uint32_t free_blocks_count;
  uint32_t free_inodes_count;
  uint32_t first_data_block;
  uint32_t log_block_size; // 1024 << Log2 block size  = block size
  uint32_t log_frag_size;
  uint32_t blocks_per_group;
  uint32_t frags_per_group;
  uint32_t inodes_per_group;
  uint32_t mtime;         // Last mount time, in POSIX time
  uint32_t wtime;         // Last write time, in POSIX time
  uint16_t mnt_count;     // # of mounts since last check
  uint16_t max_mnt_count; // # of mounts before fsck must be done
  uint16_t magic;         // 0xEF53
  uint16_t state;
  uint16_t errors;
  uint16_t minor_rev_level;
  uint32_t lastcheck;
  uint32_t checkinterval;
  uint32_t creator_os;
  uint32_t rev_level;
  uint16_t def_resuid;
  uint16_t def_resgid;
} __attribute__((packed)) superblock;

void parse_fs(char *fs);
superblock *parse_sb(char *fs);
void parse_bgdt(char *fs);

int main(int argc, char *argv[])
{
  int fd = open("./fs.bin", O_RDONLY);

  if (fd == -1)
  {
    fprintf(stderr, "Cannot open file system image.\n");
    return 1;
  }

  struct stat file_info;

  if (fstat(fd, &file_info) == -1)
  {
    fprintf(stderr, "Error getting the file size");
    return 1;
  }

  if (file_info.st_size == 0)
  {
    fprintf(stderr, "Error: File is empty, nothing to do\n");
    return 1;
  }

  printf("File system image size: %lld bytes\n", file_info.st_size);

  char *fs = mmap(
      0,
      file_info.st_size,
      PROT_READ,
      MAP_PRIVATE,
      fd,
      0);

  if (fs == MAP_FAILED)
  {
    close(fd);
    fprintf(stderr, "Error mmapping the file.\n");
    return 1;
  }

  parse_fs(fs);

  if (munmap(fs, file_info.st_size) == -1)
  {
    close(fd);
    fprintf(stderr, "Error un-mmapping the file.\n");
    return 1;
  }

  close(fd);

  return 0;
}

void parse_fs(char *fs)
{
  printf("Parsing filesystem...\n");
  parse_sb(fs);
}

superblock *parse_sb(char *fs)
{
  superblock *sp = (superblock *)((char *)fs + 1024);

  if (sp->magic != 0xef53)
  {
    printf("Bad ext2 signature.\n");
  }

  printf("Block size: %u\n", 1024 << (sp->log_block_size));

  return sp;
}

void parse_bgdt(char *fs)
{
}
