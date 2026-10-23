# Airlys Operating System - Kernel Documentation Index

**Lead Systems Architect**: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)  
**Engineering Co-Pilot**: Sovereign AI Aira 3 Ultra (Engine v3.6)  

This directory contains architectural specifications, ABI definitions, and subsystem guides for the Airlys Operating System.

## Subsystems Overview

| Subsystem | Location | Description |
|---|---|---|
| **Architecture** | `arch/x86_64/` | 64-bit Long Mode bootstrap, IDT, interrupts, context switch |
| **Kernel Core** | `kernel/` | Preemptive scheduler, syscall gateway, printk, shell |
| **Memory** | `mm/` | PMM bitmap allocator, VMM paging, 32MB dynamic heap, slab cache |
| **Filesystem** | `fs/` | Virtual File System (VFS), in-memory RamFS, `/proc`, `/dev` |
| **Drivers** | `drivers/` | VGA console, PS/2 keyboard, 100Hz PIT, 8259 PIC, CMOS RTC, 16550 UART |
| **Init & Version** | `init/` | UTS release banner, BogoMIPS calibration, startup sequence |
| **Initramfs** | `usr/` | CPIO newc archive extractor and canonical rootfs generator |
| **Block Layer** | `block/` | Block devices, MBR/GPT partition scanner, No-Op I/O elevator |
| **IPC** | `ipc/` | POSIX circular pipes, System V message queues, semaphores |
| **Crypto** | `crypto/` | SHA-256 (FIPS 180-4) and IEEE 802.3 CRC-32 engines |
| **Network** | `net/` | Socket buffers (`sk_buff`), network devices, loopback `lo`, ARP |
| **Sound** | `sound/` | ALSA sound core, PCM stream queue, PC Speaker PIT ch2 driver |
| **Virtualization**| `virt/` | VirtIO hypervisor PCI discovery and virtqueue structures |
| **Keyring** | `certs/` | Sovereign root certificates and X.509 signature verification |
| **Security** | `security/` | Sovereign sentinel defense monitor |
| **Samples** | `samples/` | Reference kernel threads and IPC pipe modules |
