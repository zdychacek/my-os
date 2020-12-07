C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c lib/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h lib/*.h)
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o}

# TODO: autogenerate
COBJS_BOOTLOADER = boot/stage2/main.o \
			boot/stage2/ext2.o \
			boot/stage2/lib.o \
			boot/stage2/vga.o \
			boot/stage2/bootconfig.o \
			boot/stage2/elf.o

# Change this if your cross-compiler is somewhere else
CC = /usr/local/i386elfgcc/bin/i386-elf-gcc
GDB = /usr/local/i386elfgcc/bin/i386-elf-gdb
LD = /usr/local/i386elfgcc/bin/i386-elf-ld
QEMU = qemu-system-i386
AS = nasm
CFLAGS = -g -ffreestanding -Wall -Wextra -fno-exceptions -m32
DISK = bin/boot.img
DISK_MOUNT_POINT = ~/mount
BOOTLOADER_STAGE1_POSITION = 0x7c00

run: image emu
debug: image emu-debug
gdb: image emu-gdb

stage1.bin: $(COBJS_BOOTLOADER)
	$(AS) -f elf -F dwarf -g boot/boot.asm -o bin/stage1.o
	$(LD) -Ttext=$(BOOTLOADER_STAGE1_POSITION) --oformat binary -o bin/stage1.bin bin/stage1.o

stage1.elf: $(COBJS_BOOTLOADER)
	$(AS) -f elf -F dwarf -g boot/boot.asm -o bin/stage1.o
	$(LD) -Ttext=$(BOOTLOADER_STAGE1_POSITION) -o bin/stage1.elf bin/stage1.o

stage2.bin: $(COBJS_BOOTLOADER)
	$(LD) -T boot/stage2/link.ld -o bin/$@ $(COBJS_BOOTLOADER) --oformat binary

stage2.elf: $(COBJS_BOOTLOADER)
	$(LD) -T boot/stage2/link.ld -o bin/$@ $(COBJS_BOOTLOADER)

kernel.elf: $(OBJ)
	$(LD) -T kernel/link.ld -o bin/$@ $^

image: stage1.bin stage2.bin kernel.elf boot.conf
	dd if=/dev/zero of=$(DISK) bs=1k count=32k
	mke2fs $(DISK)

	dd if=bin/stage1.bin of=$(DISK) conv=notrunc

	fuse-ext2 $(DISK) $(DISK_MOUNT_POINT) -o rw+

	# copy files to the filesystem
	sudo cp ./bin/stage2.bin $(DISK_MOUNT_POINT)
	sudo cp ./bin/kernel.elf $(DISK_MOUNT_POINT)/kernel
	sudo cp boot.conf $(DISK_MOUNT_POINT)

	ls -al $(DISK_MOUNT_POINT)
	umount -f $(DISK_MOUNT_POINT)

emu:
	$(QEMU) -hdb $(DISK)

emu-gdb: stage1.elf stage2.elf kernel.elf
	$(QEMU) -s -S -hdb $(DISK) &
	$(GDB) -tui -ex "target remote localhost:1234" -ex "add-symbol-file bin/stage1.elf" -ex "add-symbol-file bin/stage2.elf" -ex "add-symbol-file bin/kernel.elf"

emu-debug: stage1.elf stage2.elf kernel.elf
	$(QEMU) -s -S -hdb $(DISK)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -ffreestanding -c $< -o $@

%.o: %.asm
	$(AS) $< -f elf -o $@

%.bin: %.asm
	$(AS) $< -f bin -o $@

clean:
	rm -rf *.bin *.dis *.o
	rm -rf kernel/*.o boot/*.bin drivers/*.o boot/*.o cpu/*.o lib/*.o boot/*.o boot/stage2/*.o bin/*.elf bin/*.bin bin/*.img
