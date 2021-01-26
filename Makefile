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

QEMUFLAGS = -drive file=$(DISK),format=raw,index=1,media=disk #-d int,cpu_reset# -monitor stdio
QEMUDBGFLAGS := -s -S

STRIPFLAGS := --only-keep-debug
FUSE_EXT2 := fuse-ext2
FUSE_EXT2_FLAGS := -o rw+,allow_other,uid=$(shell id -u),gid=$(shell id -g)

OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BOOT_SRC_DIR := $(SRC_DIR)/bootloader

# BOOTSECTOR
BOOT_SECTOR_FILE := bootsector
BOOT_SECTOR_SYMBOL_FILE = $(DEBUG_DIR)/$(BOOT_SECTOR_FILE).$(DEBUG_SYMBOL_EXT)
BOOT_SECTOR_RELEASE_FILE = $(RELEASE_DIR)/$(BOOT_SECTOR_FILE).$(BINARY_EXT)
BOOT_SECTOR_SRC_DIR := $(BOOT_SRC_DIR)/bootsector
BOOT_SECTOR_SRC_ASM := $(wildcard $(BOOT_SECTOR_SRC_DIR)/*.asm)
BOOT_SECTOR_OBJ := $(BOOT_SECTOR_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
BOOT_SECTOR_POSITION := 0x7c00

# BOOTLOADER STAGE 1
BOOT_STAGE1_FILE := stage1
BOOT_STAGE1_SYMBOL_FILE = $(DEBUG_DIR)/$(BOOT_STAGE1_FILE).$(DEBUG_SYMBOL_EXT)
BOOT_STAGE1_RELEASE_FILE = $(RELEASE_DIR)/$(BOOT_STAGE1_FILE).$(BINARY_EXT)
BOOT_STAGE1_SRC_DIR := $(BOOT_SRC_DIR)/stage1
BOOT_STAGE1_SRC_ASM := $(wildcard $(BOOT_STAGE1_SRC_DIR)/*.asm)
BOOT_STAGE1_OBJ := $(BOOT_STAGE1_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)
BOOT_STAGE1_POSITION := 0x5000

# BOOTLOADER STAGE 2
BOOT_STAGE2_FILE := stage2
BOOT_STAGE2_SYMBOL_FILE := $(DEBUG_DIR)/$(BOOT_STAGE2_FILE).$(DEBUG_SYMBOL_EXT)
BOOT_STAGE2_RELEASE_FILE := $(RELEASE_DIR)/$(BOOT_STAGE2_FILE).$(BINARY_EXT)
BOOT_STAGE2_SRC_DIR := $(BOOT_SRC_DIR)/stage2
BOOT_STAGE2_SRC_ASM := $(wildcard $(BOOT_STAGE2_SRC_DIR)/*.asm)
BOOT_STAGE2_SRC_C := $(wildcard $(BOOT_STAGE2_SRC_DIR)/*.c)

BOOT_STAGE2_OBJ := $(BOOT_STAGE2_SRC_C:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) \
				$(BOOT_STAGE2_SRC_ASM:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.o)

BOOT_STAGE2_LINK_FILE := $(BOOT_STAGE2_SRC_DIR)/stage2.ld

BOOT_STAGE2_LINK = $(LD) \
	-m elf_i386 \
	-T $(BOOT_STAGE2_LINK_FILE) \
	-L$(RELEASE_DIR) \
	-l$(subst lib,,$(LIB_FILE)) \
	-o $@ $(filter %/main.o, $^) $(filter-out %/main.o, $^)

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
DISK_NAME := disk.img
DISK := $(RELEASE_DIR)/$(DISK_NAME)
DISK_SIZE := 256k
DISK_MOUNT_DIR := ./mount

BOOT_CONF := boot.conf

DIR_SENTINEL = @mkdir -p $(@D)

.PHONY: run compile gdb dbg clean

run: $(DISK)
	$(QEMU) $(QEMUFLAGS)

compile: $(BOOT_SECTOR_RELEASE_FILE) \
	$(BOOT_STAGE1_RELEASE_FILE) \
	$(BOOT_STAGE2_RELEASE_FILE) \
	$(KERNEL_RELEASE_FILE)

dbg: $(DISK) \
	$(BOOT_SECTOR_SYMBOL_FILE) \
	$(BOOT_STAGE1_SYMBOL_FILE) \
	$(BOOT_STAGE2_SYMBOL_FILE) \
	$(KERNEL_SYMBOL_FILE)
	$(QEMU) \
	$(QEMUDBGFLAGS) \
	$(QEMUFLAGS)

gdb: $(DISK) \
	$(BOOT_SECTOR_SYMBOL_FILE) \
	$(BOOT_STAGE1_SYMBOL_FILE) \
	$(BOOT_STAGE2_SYMBOL_FILE) \
	$(KERNEL_SYMBOL_FILE)

	$(QEMU) $(QEMUDBGFLAGS) $(QEMUFLAGS) &
	$(GDB) \
		-tui \
		-ex "target remote localhost:1234" \
		-ex "add-symbol-file $(BOOT_SECTOR_SYMBOL_FILE)" \
		-ex "add-symbol-file $(BOOT_STAGE1_SYMBOL_FILE)" \
		-ex "add-symbol-file $(BOOT_STAGE2_SYMBOL_FILE)" \
		-ex "add-symbol-file $(KERNEL_SYMBOL_FILE)"

# BOOT SECTOR
$(BOOT_SECTOR_RELEASE_FILE): $(BOOT_SECTOR_OBJ) | $(RELEASE_DIR)
	$(LD) \
		-m elf_i386 \
		-Ttext=$(BOOT_SECTOR_POSITION) \
		--oformat binary \
		-o $@ $^

# BOOTLOADER STAGE 1
$(BOOT_STAGE1_RELEASE_FILE): $(BOOT_STAGE1_OBJ) | $(RELEASE_DIR)
	$(LD) \
		-m elf_i386 \
		-Ttext=$(BOOT_STAGE1_POSITION) \
		--oformat binary \
		-o $@ $^

# BOOTLOADER STAGE 2
$(BOOT_STAGE2_RELEASE_FILE): $(BOOT_STAGE2_OBJ) $(LIB_RELEASE_FILE) | $(RELEASE_DIR)
	$(BOOT_STAGE2_LINK) --oformat binary

$(BOOT_SECTOR_SYMBOL_FILE): $(BOOT_SECTOR_OBJ) | $(DEBUG_DIR)
	$(LD) \
		-m elf_i386 \
		-Ttext=$(BOOT_SECTOR_POSITION) \
		-o $@ $^
	$(STRIP) $(STRIPFLAGS) $@

$(BOOT_STAGE1_SYMBOL_FILE): $(BOOT_STAGE1_OBJ) | $(DEBUG_DIR)
	$(LD) \
		-m elf_i386 \
		-Ttext=$(BOOT_STAGE1_POSITION) \
		-o $@ $^
	$(STRIP) $(STRIPFLAGS) $@

$(BOOT_STAGE2_SYMBOL_FILE): $(BOOT_STAGE2_OBJ) $(LIB_RELEASE_FILE) | $(DEBUG_DIR)
	$(BOOT_STAGE2_LINK)
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
	$(BOOT_CONF) \
	$(BOOT_SECTOR_RELEASE_FILE) \
	$(BOOT_STAGE1_RELEASE_FILE) \
	$(BOOT_STAGE2_RELEASE_FILE) \
	$(KERNEL_RELEASE_FILE)

	dd if=/dev/zero of=$@ bs=1 count=$(DISK_SIZE)
	mke2fs $@

	dd if=$(BOOT_SECTOR_RELEASE_FILE) of=$@ conv=notrunc

	$(FUSE_EXT2) $@ $(DISK_MOUNT_DIR) $(FUSE_EXT2_FLAGS)
	sudo chown -R $(shell id -u):$(shell id -g) $(DISK_MOUNT_DIR)

	cp $(BOOT_STAGE1_RELEASE_FILE) $(DISK_MOUNT_DIR)
	cp $(BOOT_STAGE2_RELEASE_FILE) $(DISK_MOUNT_DIR)
	cp $(KERNEL_RELEASE_FILE) $(DISK_MOUNT_DIR)
	cp $(BOOT_CONF) $(DISK_MOUNT_DIR)

	ls -al $(DISK_MOUNT_DIR)
	umount -f $(DISK_MOUNT_DIR)

clean:
	@rm -rfv $(RELEASE_DIR) \
		$(DEBUG_DIR) \
		$(OBJ_DIR) \
		$(DISK_MOUNT_DIR)

-include $(OBJ:.o=.d)
