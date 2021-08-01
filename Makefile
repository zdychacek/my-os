ifeq ($(MYOS),)
$(error Unsupported environment! Run in Docker.)
endif

CXX := x86_64-elf-g++
LD := x86_64-elf-ld
STRIP := x86_64-elf-strip
AS := nasm

INC_DIR := src
SRC_DIR := src
OBJ_DIR := obj
RELEASE_DIR := release

BUILD_WARNING := \
	-Wall \
	-Wextra  \
	-Werror \
	# some exceptions
	-Wno-error=unused-variable \
	-Wno-error=unused-function \
	-Wno-error=unused-parameter

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

STRIPFLAGS := --only-keep-debug

KERNEL_FILE := kernel
SYMBOL_FILE := $(RELEASE_DIR)/$(KERNEL_FILE).sym
RELEASE_FILE := $(RELEASE_DIR)/$(KERNEL_FILE).bin
ISO := $(RELEASE_DIR)/$(KERNEL_FILE).iso

SRC_ASM := $(shell find $(SRC_DIR) -name *.s)
SRC_CPP := $(shell find $(SRC_DIR) -name *.cpp)

OBJ := $(SRC_CPP:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) $(SRC_ASM:$(SRC_DIR)/%.s=$(OBJ_DIR)/%.o)

LINK_FILE = $(SRC_DIR)/kernel.ld
KERNEL_LINK = $(LD) -n -T $(LINK_FILE) -o $@ $^
DIR_SENTINEL = @mkdir -p $(@D)

.PHONY: all clean

all: $(ISO)

$(RELEASE_FILE): $(OBJ) | $(RELEASE_DIR)
	@echo [LD] $<
	@$(KERNEL_LINK)

$(SYMBOL_FILE): $(OBJ) | $(RELEASE_DIR)
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

$(RELEASE_DIR) $(OBJ_DIR):
	@mkdir -p $@

$(ISO): $(RELEASE_FILE)
	cp $(RELEASE_FILE) grub/boot/kernel.bin && \
	grub-mkrescue /usr/lib/grub/i386-pc -o $(ISO) grub

clean:
	@rm -rfv $(RELEASE_DIR) $(OBJ_DIR) grub/boot/kernel.bin

-include $(OBJ:.o=.d)
