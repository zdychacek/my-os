
#  https://stackoverflow.com/questions/30573481/path-include-and-src-directory-makefile/30602701

SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj
RELEASE_DIR := release
DEBUG_DIR := debug
DEBUG_SYMBOL_EXT := sym
BINARY_EXT := bin

CC := i386-elf-gcc
LD := i386-elf-ld
STRIP := i386-elf-strip
AS := nasm
QEMU := qemu-system-i386
GDB = i386-elf-gdb
CFLAGS := -I$(INC_DIR) -MMD -MP -g -ffreestanding -Wall -Wextra -fno-exceptions -m32
STRIPFLAGS := --only-keep-debug

OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BOOT_SRC_DIR := $(SRC_DIR)/boot

# BOOTLOADER STAGE 1
BOOT_STAGE1_FILE := stage1
BOOT_STAGE1_SYMBOL_FILE = $(DEBUG_DIR)/$(BOOT_STAGE1_FILE).$(DEBUG_SYMBOL_EXT)
BOOT_STAGE1_RELEASE_FILE = $(RELEASE_DIR)/$(BOOT_STAGE1_FILE).$(BINARY_EXT)
BOOT_STAGE1_SRC_ASM := $(wildcard $(BOOT_SRC_DIR)/*.asm)
BOOT_STAGE1_OBJ := $(BOOT_STAGE1_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
BOOT_STAGE1_POSITION := 0x7c00

# BOOTLOADER STAGE 2
BOOT_STAGE2_FILE := stage2
BOOT_STAGE2_SYMBOL_FILE := $(DEBUG_DIR)/$(BOOT_STAGE2_FILE).$(DEBUG_SYMBOL_EXT)
BOOT_STAGE2_RELEASE_FILE := $(RELEASE_DIR)/$(BOOT_STAGE2_FILE).$(BINARY_EXT)
BOOT_STAGE2_SRC_DIR := $(BOOT_SRC_DIR)/stage2
BOOT_STAGE2_SRC_ASM := $(wildcard $(BOOT_STAGE2_SRC_DIR)/*.asm)
BOOT_STAGE2_SRC_C := $(wildcard $(BOOT_STAGE2_SRC_DIR)/*.c)
BOOT_STAGE2_OBJ := $(BOOT_STAGE2_SRC_C:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) \
				$(BOOT_STAGE2_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
BOOT_STAGE2_LINK_FILE := $(BOOT_STAGE2_SRC_DIR)/link.ld
BOOT_STAGE2_LINK = $(LD) -T $(BOOT_STAGE2_LINK_FILE) -o $@ $(filter %/main.o, $^) $(filter-out %/main.o, $^)

# KERNEL
KERNEL_FILE := kernel
KERNEL_SYMBOL_FILE := $(DEBUG_DIR)/$(KERNEL_FILE).$(DEBUG_SYMBOL_EXT)
KERNEL_RELEASE_FILE := $(RELEASE_DIR)/$(KERNEL_FILE).$(BINARY_EXT)
KERNEL_SRC_SUBDIRS := arch-x86 drivers kernel lib
KERNEL_SRC_ASM := $(foreach dir,$(KERNEL_SRC_SUBDIRS),$(wildcard $(SRC_DIR)/$(dir)/*.asm))
KERNEL_SRC_C := $(foreach dir,$(KERNEL_SRC_SUBDIRS),$(wildcard $(SRC_DIR)/$(dir)/*.c))
KERNEL_OBJ := $(KERNEL_SRC_C:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) \
       	$(KERNEL_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
KERNEL_LINK_FILE = $(SRC_DIR)/kernel/link.ld
KERNEL_LINK = $(LD) -T $(KERNEL_LINK_FILE) -o $@ $^

# DISK
DISK_NAME := boot.img
DISK := $(RELEASE_DIR)/$(DISK_NAME)
DISK_SIZE := 256k
DISK_MOUNT_DIR := ./mount

BOOT_CONF := boot.conf

DIR_SENTINEL = @mkdir -p $(@D)

.PHONY: run gdb dbg clean

run: $(DISK)
	$(QEMU) -hdb $(DISK)

dbg: $(DISK) $(BOOT_STAGE1_SYMBOL_FILE) $(BOOT_STAGE2_SYMBOL_FILE) $(KERNEL_SYMBOL_FILE)
	$(QEMU) -s -S -hdb $(DISK)

gdb: $(DISK) $(BOOT_STAGE1_SYMBOL_FILE) $(BOOT_STAGE2_SYMBOL_FILE) $(KERNEL_SYMBOL_FILE)
	$(QEMU) -s -S -hdb $(DISK) &
	$(GDB) \
		-tui \
		-ex "target remote localhost:1234" \
		-ex "add-symbol-file $(BOOT_STAGE1_SYMBOL_FILE)" \
		-ex "add-symbol-file $(BOOT_STAGE2_SYMBOL_FILE)" \
		-ex "add-symbol-file $(KERNEL_SYMBOL_FILE)"

# BOOTLOADER STAGE 1
$(BOOT_STAGE1_RELEASE_FILE): $(BOOT_STAGE1_OBJ) | $(RELEASE_DIR)
	$(LD) -Ttext=$(BOOT_STAGE1_POSITION) --oformat binary -o $@ $^

$(BOOT_STAGE1_SYMBOL_FILE): $(BOOT_STAGE1_OBJ) | $(DEBUG_DIR)
	$(LD) -Ttext=$(BOOT_STAGE1_POSITION) -o $@ $^
	$(STRIP) $(STRIPFLAGS) $@

# BOOTLOADER STAGE 2
$(BOOT_STAGE2_RELEASE_FILE): $(BOOT_STAGE2_OBJ) | $(RELEASE_DIR)
	$(BOOT_STAGE2_LINK) --oformat binary

$(BOOT_STAGE2_SYMBOL_FILE): $(BOOT_STAGE2_OBJ) | $(DEBUG_DIR)
	$(BOOT_STAGE2_LINK)
	$(STRIP) $(STRIPFLAGS) $@

# KERNEL
$(KERNEL_RELEASE_FILE): $(KERNEL_OBJ) | $(RELEASE_DIR)
	$(KERNEL_LINK)
	$(STRIP) $@

$(KERNEL_SYMBOL_FILE): $(KERNEL_OBJ) | $(DEBUG_DIR)
	$(KERNEL_LINK)
	$(STRIP) $(STRIPFLAGS) $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	$(DIR_SENTINEL)
	$(AS) $< -g -f elf -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(DIR_SENTINEL)
	$(CC) $(CFLAGS) -c $< -o $@

$(RELEASE_DIR) $(DEBUG_DIR) $(OBJ_DIR) $(DISK_MOUNT_DIR):
	@mkdir -p $@

# DISK
$(DISK): $(DISK_MOUNT_DIR) $(BOOT_CONF) $(BOOT_STAGE1_RELEASE_FILE) $(BOOT_STAGE2_RELEASE_FILE) $(KERNEL_RELEASE_FILE)
	dd if=/dev/zero of=$@ bs=1 count=$(DISK_SIZE)
	mke2fs $@

	dd if=$(BOOT_STAGE1_RELEASE_FILE) of=$@ conv=notrunc

	fuse-ext2 $@ $(DISK_MOUNT_DIR) -o rw+

	sudo cp $(BOOT_STAGE2_RELEASE_FILE) $(DISK_MOUNT_DIR)
	sudo cp $(KERNEL_RELEASE_FILE) $(DISK_MOUNT_DIR)
	sudo cp $(BOOT_CONF) $(DISK_MOUNT_DIR)

	ls -al $(DISK_MOUNT_DIR)
	umount -f $(DISK_MOUNT_DIR)

clean:
	@rm -rfv $(RELEASE_DIR) $(DEBUG_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
