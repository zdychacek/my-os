
  # Virtual Memory Layout

| Start       | End        | Length   | Description
|------------ |------------|----------|-------------
| 0x0         | 0xbfffffff | 3 GB     | user space
| 0xc0000000  | 0xc03fffff | 4 MB     | kernel code and data
| 0xc0400000  | 0xff3fffff | 1007 MB  | kernel heap
| 0xff400000  | 0xffbfffff | 8 MB     | framebuffer data space
| 0xffc00000  | 0xffffffff | 4 MB     | page directory is allocated into itself, used to access page tables
