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
               kernel/printk.c \
               kernel/airalang.c \
               kernel/compositor.c \
               kernel/apps.c \
               mm/pmm.c \
               mm/vmm.c \
               mm/heap.c \
               mm/slab.c \
               fs/vfs.c \
               fs/ramfs.c \
               fs/procfs.c \
               fs/devfs.c \
               drivers/vga.c \
               drivers/keyboard.c \
               drivers/timer.c \
               drivers/pic.c \
               drivers/rtc.c \
               drivers/serial.c \
               drivers/pci.c \
               drivers/vbe.c \
               drivers/font.c \
               security/sentinel.c \
               lib/string.c \
               arch/x86_64/idt.c \
               usr/initramfs.c \
               init/version.c \
               init/calibrate.c \
               block/blk-core.c \
               block/partition.c \
               block/elevator.c \
               ipc/pipe.c \
               ipc/sem.c \
               ipc/msg.c \
               crypto/sha256.c \
               crypto/crc32.c \
               crypto/api.c \
               net/skbuff.c \
               net/netdev.c \
               net/arp.c \
               net/ipv4.c \
               net/icmp.c \
               net/udp.c \
               net/tcp.c \
               net/dhcp.c \
               sound/core.c \
               sound/pcspkr.c \
               virt/virtio.c \
               certs/keyring.c \
               samples/hello_kernel.c \
               samples/sample_pipe.c

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
