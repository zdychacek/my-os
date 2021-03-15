# Makes:
#   1. Bootloader (Stage 1 and Stage 2)
#   2. Kernel
#   3. `libstd` static library

SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj
RELEASE_DIR := release
DEBUG_DIR := debug
DEBUG_SYMBOL_EXT := sym
BINARY_EXT := bin

CC := x86_64-elf-gcc
LD := x86_64-elf-ld
STRIP := x86_64-elf-strip
AR := x86_64-elf-ar
AS := nasm
QEMU := qemu-system-x86_64
GDB = gdb
ARFLAGS := rcs
CFLAGS := -I$(INC_DIR) \
	-MMD \
	-MP \
	-g \
	-ffreestanding \
	-Wall \
	-Wextra \
	-fno-exceptions \
	-m32

QEMUFLAGS = -drive file=$(DISK),format=raw,index=1,media=disk -monitor stdio #-accel hvf
QEMUDBGFLAGS := -s -S

STRIPFLAGS := --only-keep-debug
FUSE_EXT2 := fuse-ext2
FUSE_EXT2_FLAGS := -o rw+

OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BOOT_SRC_DIR := $(SRC_DIR)/boot

# MBR
MBR_FILE := mbr
MBR_SYMBOL_FILE = $(DEBUG_DIR)/$(MBR_FILE).$(DEBUG_SYMBOL_EXT)
MBR_RELEASE_FILE = $(RELEASE_DIR)/$(MBR_FILE).$(BINARY_EXT)
MBR_SRC_DIR := $(BOOT_SRC_DIR)
MBR_SRC_ASM := $(MBR_SRC_DIR)/mbr.asm
MBR_OBJ := $(MBR_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
MBR_POSITION := 0x0600

# VOLUME BOOT RECORD
VBR_FILE := vbr
VBR_SYMBOL_FILE = $(DEBUG_DIR)/$(VBR_FILE).$(DEBUG_SYMBOL_EXT)
VBR_RELEASE_FILE = $(RELEASE_DIR)/$(VBR_FILE).$(BINARY_EXT)
VBR_SRC_DIR := $(BOOT_SRC_DIR)
VBR_SRC_ASM := $(VBR_SRC_DIR)/vbr.asm
VBR_OBJ := $(VBR_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
VBR_POSITION := 0x7c00

# BOOTLOADER STAGE 2
BOOT_STAGE2_FILE := stage2
BOOT_STAGE2_SYMBOL_FILE = $(DEBUG_DIR)/$(BOOT_STAGE2_FILE).$(DEBUG_SYMBOL_EXT)
BOOT_STAGE2_RELEASE_FILE = $(RELEASE_DIR)/$(BOOT_STAGE2_FILE).$(BINARY_EXT)
BOOT_STAGE2_SRC_DIR := $(BOOT_SRC_DIR)/stage2
BOOT_STAGE2_SRC_ASM := $(wildcard $(BOOT_STAGE2_SRC_DIR)/*.asm)
BOOT_STAGE2_OBJ := $(BOOT_STAGE2_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
BOOT_STAGE2_POSITION := 0x5000

# LIBRARY
LIB_FILE := libstd
LIB_SYMBOL_FILE := $(RELEASE_DIR)/$(LIB_FILE).$(DEBUG_SYMBOL_EXT)
LIB_RELEASE_FILE := $(RELEASE_DIR)/$(LIB_FILE).a
LIB_SRC_DIR := $(SRC_DIR)/lib
LIB_SRC_C := $(wildcard $(LIB_SRC_DIR)/*.c)
LIB_OBJ := $(LIB_SRC_C:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# KERNEL
KERNEL_FILE := kernel
KERNEL_SYMBOL_FILE := $(DEBUG_DIR)/$(KERNEL_FILE).$(DEBUG_SYMBOL_EXT)
KERNEL_RELEASE_FILE := $(RELEASE_DIR)/$(KERNEL_FILE).$(BINARY_EXT)
KERNEL_SRC_DIR := $(SRC_DIR)/kernel
KERNEL_SRC_ASM := $(wildcard $(KERNEL_SRC_DIR)/*.asm) $(wildcard $(KERNEL_SRC_DIR)/**/*.asm)

KERNEL_SRC_C := $(wildcard $(KERNEL_SRC_DIR)/*.c) \
	$(wildcard $(KERNEL_SRC_DIR)/**/*.c)

KERNEL_OBJ := $(KERNEL_SRC_C:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) \
				$(KERNEL_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)

KERNEL_LINK_FILE = $(KERNEL_SRC_DIR)/kernel.ld

KERNEL_LINK = $(LD) \
	-m elf_i386 \
	-T $(KERNEL_LINK_FILE) \
	-L$(RELEASE_DIR) \
	-l$(subst lib,,$(LIB_FILE)) \
	-o $@ $^

# DISK
DISK_NAME := disk.$(BINARY_EXT)
DISK := $(RELEASE_DIR)/$(DISK_NAME)
DISK_SIZE_MB := 16
DISK_MOUNT_DIR := ./mount

FS_IMAGE := $(RELEASE_DIR)/fs.$(BINARY_EXT)
FS_START_SECTOR := 32697
BOOT_CONF := boot.conf

DIR_SENTINEL = @mkdir -p $(@D)

.PHONY: run compile gdb dbg clean

run: $(DISK)
	$(QEMU) $(QEMUFLAGS)

compile: $(MBR_RELEASE_FILE) \
	$(VBR_RELEASE_FILE) \
	$(BOOT_STAGE2_RELEASE_FILE) \
	$(KERNEL_RELEASE_FILE)

dbg: $(DISK) \
	$(MBR_SYMBOL_FILE) \
	$(VBR_SYMBOL_FILE) \
	$(BOOT_STAGE2_SYMBOL_FILE) \
	$(KERNEL_SYMBOL_FILE)
	$(QEMU) \
	$(QEMUDBGFLAGS) \
	$(QEMUFLAGS)

gdb: $(DISK) \
	$(MBR_SYMBOL_FILE) \
	$(VBR_SYMBOL_FILE) \
	$(BOOT_STAGE2_SYMBOL_FILE) \
	$(KERNEL_SYMBOL_FILE)

	$(QEMU) $(QEMUDBGFLAGS) $(QEMUFLAGS) &
	$(GDB) \
		-tui \
		-ex "target remote localhost:1234" \
		-ex "add-symbol-file $(MBR_SYMBOL_FILE)" \
		-ex "add-symbol-file $(VBR_SYMBOL_FILE)" \
		-ex "add-symbol-file $(BOOT_STAGE2_SYMBOL_FILE)" \
		-ex "add-symbol-file $(KERNEL_SYMBOL_FILE)" \

# MBR
$(MBR_RELEASE_FILE): $(MBR_OBJ) | $(RELEASE_DIR)
	$(LD) -m elf_i386 -Ttext=$(MBR_POSITION) --oformat binary -o $@ $^

$(MBR_SYMBOL_FILE): $(MBR_OBJ) | $(DEBUG_DIR)
	$(LD) -m elf_i386 -Ttext=$(MBR_POSITION) -o $@ $^
	$(STRIP) $(STRIPFLAGS) $@

# BOOT SECTOR
$(VBR_RELEASE_FILE): $(VBR_OBJ) | $(RELEASE_DIR)
	$(LD) -m elf_i386 -Ttext=$(VBR_POSITION) --oformat binary -o $@ $^

$(VBR_SYMBOL_FILE): $(VBR_OBJ) | $(DEBUG_DIR)
	$(LD) -m elf_i386 -Ttext=$(VBR_POSITION) -o $@ $^
	$(STRIP) $(STRIPFLAGS) $@

# BOOTLOADER STAGE 1
$(BOOT_STAGE2_RELEASE_FILE): $(BOOT_STAGE2_OBJ) | $(RELEASE_DIR)
	$(LD) -m elf_i386 -Ttext=$(BOOT_STAGE2_POSITION) --oformat binary -o $@ $^

$(BOOT_STAGE2_SYMBOL_FILE): $(BOOT_STAGE2_OBJ) | $(DEBUG_DIR)
	$(LD) -m elf_i386 -Ttext=$(BOOT_STAGE2_POSITION) -o $@ $^
	$(STRIP) $(STRIPFLAGS) $@

# KERNEL
$(KERNEL_RELEASE_FILE): $(KERNEL_OBJ) $(LIB_RELEASE_FILE) | $(RELEASE_DIR)
	$(KERNEL_LINK)
	$(STRIP) $@

$(KERNEL_SYMBOL_FILE): $(KERNEL_OBJ) $(LIB_RELEASE_FILE) | $(DEBUG_DIR)
	$(KERNEL_LINK)
	$(STRIP) $(STRIPFLAGS) $@

# LIBRARY
$(LIB_RELEASE_FILE): $(LIB_OBJ) | $(RELEASE_DIR)
	$(AR) $(ARFLAGS) $@ $(LIB_OBJ)

# TODO(ondrej): take *.inc files into account
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	$(DIR_SENTINEL)
	$(AS) $< -g -f elf32 -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(DIR_SENTINEL)
	$(CC) $(CFLAGS) -c $< -o $@

$(RELEASE_DIR) $(DEBUG_DIR) $(OBJ_DIR) $(DISK_MOUNT_DIR):
	@mkdir -p $@

# DISK
$(DISK): $(DISK_MOUNT_DIR) \
	$(MBR_RELEASE_FILE) \
	$(VBR_RELEASE_FILE) \
	$(FS_IMAGE)

	@echo "--- Creating empty disk image ---"
	@dd if=/dev/zero of=$@ bs=1m count=$(DISK_SIZE_MB)
	@LOOPBACK=$$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount $@ | head -n 1 | cut -d " " -f 1); \
	echo "--- Disk image mounted at $$LOOPBACK ---"; \
	echo "--- Creating ext2 partiton ---"; \
	diskutil partitionDisk $$LOOPBACK MBR fuse-ext2 "My OS" 100%; \
	echo "--- Writing master boot record ---"; \
	./tools/write_mbr $(MBR_RELEASE_FILE) $@; \
	echo "--- Writing filesystem ---"; \
	dd if=$(FS_IMAGE) of=$@ bs=512 count=$(FS_START_SECTOR) seek=63 conv=notrunc; \
	echo "--- Writing partition boot sector ---"; \
	dd if=$(VBR_RELEASE_FILE) of=$@ bs=512 count=$(FS_START_SECTOR) seek=63 conv=notrunc; \
	echo "--- Making ext2 partiton bootable ---"; \
	./tools/set_byte $@ 446 0x80; \
	echo "--- Unmounting disk image ---"; \
	hdiutil detach $$LOOPBACK

$(FS_IMAGE): $(BOOT_CONF) \
	$(BOOT_STAGE2_RELEASE_FILE) \
	$(KERNEL_RELEASE_FILE)

	dd if=/dev/zero of=$@ bs=512 count=$(FS_START_SECTOR)
	mke2fs $@
	$(FUSE_EXT2) $@ $(DISK_MOUNT_DIR) $(FUSE_EXT2_FLAGS)

	sudo cp $(BOOT_STAGE2_RELEASE_FILE) $(DISK_MOUNT_DIR)
	sudo cp $(KERNEL_RELEASE_FILE) $(DISK_MOUNT_DIR)
	sudo cp $(BOOT_CONF) $(DISK_MOUNT_DIR)

	umount -f $(DISK_MOUNT_DIR)

flash: $(DISK)
	@echo "Which disk? (default: /dev/disk2)" && read answer && answer=$${answer:-/dev/disk2}; \
	sudo dd if=$(DISK) of=$${answer} bs=1m count=$(DISK_SIZE_MB)

clean:
	@rm -rfv $(RELEASE_DIR) \
		$(DEBUG_DIR) \
		$(OBJ_DIR) \
		$(DISK_MOUNT_DIR)

-include $(OBJ:.o=.d)
