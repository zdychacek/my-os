CXX := x86_64-elf-gcc
LD := x86_64-elf-ld
STRIP := x86_64-elf-strip
AS := nasm
QEMU := qemu-system-x86_64
GDB := gdb

INC_DIR := src
SRC_DIR := src
OBJ_DIR := obj
RELEASE_DIR := release
DEBUG_DIR := debug

BUILD_WARNING := \
	-Wall \
	-Wextra  \
	-Werror

CXXFLAGS := \
	-I$(INC_DIR) \
	-m64 \
	-std=c++20 \
	-MMD \
	-MP \
	-g \
	-fno-rtti \
	-fno-exceptions \
	-ffreestanding \
	-nostdlib \
	-Wnon-virtual-dtor \
	-Woverloaded-virtual \
	-mcmodel=large \
	-mno-red-zone \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-D__TEST__ \
	$(BUILD_WARNING)

QEMUFLAGS = -cdrom release/kernel.iso -serial stdio -m 128m # -d int -no-reboot -no-shutdown # -monitor stdio -accel hvf
QEMUDBGFLAGS := -s -S

STRIPFLAGS := --only-keep-debug

KERNEL_FILE := kernel
SYMBOL_FILE := $(DEBUG_DIR)/$(KERNEL_FILE).sym
RELEASE_FILE := $(RELEASE_DIR)/$(KERNEL_FILE).bin
ISO := $(RELEASE_DIR)/$(KERNEL_FILE).iso

SRC_ASM := $(shell find $(SRC_DIR) -name *.s)
SRC_CPP := $(shell find $(SRC_DIR) -name *.cpp)

OBJ := $(SRC_CPP:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) $(SRC_ASM:$(SRC_DIR)/%.s=$(OBJ_DIR)/%.o)

LINK_FILE = $(SRC_DIR)/kernel.ld
KERNEL_LINK = $(LD) -n -T $(LINK_FILE) -o $@ $^
DIR_SENTINEL = @mkdir -p $(@D)

.PHONY: run compile gdb dbg clean create-iso

qemu: $(ISO)
	$(QEMU) $(QEMUFLAGS)

compile: $(ISO)

vbox: $(ISO)
	./scripts/run-virtual-box.sh

dbg: $(ISO) $(SYMBOL_FILE)
	$(QEMU) $(QEMUDBGFLAGS) $(QEMUFLAGS)

gdb: $(ISO) $(SYMBOL_FILE)
	$(QEMU) $(QEMUDBGFLAGS) $(QEMUFLAGS) &
	$(GDB) \
		-tui \
		-ex "target remote localhost:1234" \
		-ex "add-symbol-file $(SYMBOL_FILE)" \

$(RELEASE_FILE): $(OBJ) | $(RELEASE_DIR)
	@echo [LD] $<
	@$(KERNEL_LINK)

$(SYMBOL_FILE): $(OBJ) | $(DEBUG_DIR)
	@echo [KERNEL:debug] [LD] $<
	@$(KERNEL_LINK)
	@$(STRIP) $(STRIPFLAGS) $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s | $(OBJ_DIR)
	$(DIR_SENTINEL)
	@echo [ASM] $<
	@$(AS) $< -g -f elf64 -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(DIR_SENTINEL)
	@echo [CXX] $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(RELEASE_DIR) $(DEBUG_DIR) $(OBJ_DIR):
	@mkdir -p $@

$(ISO): $(RELEASE_FILE)
	cp $(RELEASE_FILE) grub/boot/kernel.bin && \
	docker run --rm -it -v "$${PWD}":/root/env myos-buildenv grub-mkrescue /usr/lib/grub/i386-pc -o $(ISO) grub

flash: $(ISO)
	@echo "Which disk? (default: /dev/disk2)" && read answer && answer=$${answer:-/dev/disk2}; \
	sudo dd if=$(ISO) of=$${answer} bs=1m

clean:
	@rm -rfv $(RELEASE_DIR) $(DEBUG_DIR) $(OBJ_DIR) grub/boot/kernel.bin

-include $(OBJ:.o=.d)
