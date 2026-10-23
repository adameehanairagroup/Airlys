# Airlys Operating System Kernel - Makefile
# Engineered by Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
# Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)

TARGET      := x86_64-unknown-none-elf
CC          := clang
LD          := ld.lld
AS          := clang

CFLAGS      := --target=$(TARGET) -ffreestanding -fno-builtin -fno-stack-protector \
               -mno-red-zone -nostdlib -mno-mmx -mno-sse -mno-sse2 \
               -Wall -Wextra -Werror -Iinclude -O2 -g

ASFLAGS     := --target=$(TARGET) -c -g
LDFLAGS     := -m elf_x86_64 -nostdlib -static -T linker.ld

BUILD_DIR   := build
BIN_DIR     := bin
KERNEL_BIN  := $(BIN_DIR)/airlys.elf

# Sources
C_SRCS      := kernel/kmain.c \
               kernel/panic.c \
               kernel/sched.c \
               kernel/syscall.c \
               kernel/shell.c \
               mm/pmm.c \
               mm/vmm.c \
               mm/heap.c \
               fs/vfs.c \
               fs/ramfs.c \
               drivers/vga.c \
               drivers/keyboard.c \
               drivers/timer.c \
               drivers/pic.c \
               security/sentinel.c \
               lib/string.c \
               arch/x86_64/idt.c

ASM_SRCS    := arch/x86_64/boot.s \
               arch/x86_64/interrupts.s \
               arch/x86_64/switch.s

# Objects
OBJS        := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS)) \
               $(patsubst %.s, $(BUILD_DIR)/%.o, $(ASM_SRCS))

.PHONY: all clean info check

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJS) | $(BIN_DIR)
	@echo " [LD] Linking $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo " [OK] Airlys Kernel built successfully: $@"

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo " [CC] Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo " [AS] Assembling $<"
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

info: $(KERNEL_BIN)
	@echo "================ Airlys Kernel Binary Info ================"
	@file $(KERNEL_BIN)
	@size $(KERNEL_BIN) || true

clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo " [CLEAN] Removed build artifacts."
