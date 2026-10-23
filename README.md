# 🌌 AIRLYS: AI-Native Operating System Kernel

<div align="center">

<p align="center">
  <img src="assets/airlys-logo.png" width="700" alt="Airlys Sovereign Eagle Crest - The Sovereign Kernel" style="border-radius: 12px; box-shadow: 0 8px 32px rgba(0, 242, 254, 0.2);" />
</p>

```
     _    ___ ____  _  __   ______  
    / \  |_ _|  _ \| | \ \ / / ___| 
   / _ \  | || |_) | |  \ V /\___ \ 
  / ___ \ | ||  _ <| |___| |  ___) |
 /_/   \_\___|_| \_\_____|_| |____/  
```

### ⚡ 64-Bit Monolithic Operating System Kernel Engineered from First Principles
**An Open-Source Masterpiece by Aira Group of Technology**

<br/>

[![Architecture](https://img.shields.io/badge/Architecture-x86__64_Long_Mode-00f2fe?style=for-the-badge&logo=cpu&logoColor=black)](#)
[![Standard](https://img.shields.io/badge/Bootloader-Multiboot_1_%26_2-00C853?style=for-the-badge&logo=gnubash&logoColor=white)](#)
[![Status](https://img.shields.io/badge/Build-PASSING_ZERO_WARNINGS-brightgreen?style=for-the-badge&logo=checkmarx&logoColor=white)](#)
[![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)](#)
[![Founder](https://img.shields.io/badge/Founder_&_Architect-Er._Adam_Eehan-00f2fe?style=for-the-badge&logo=github&logoColor=black)](https://github.com/adameehanairagroup)

<br/>

```text
"We do not build software to follow systems; we build operating systems to define reality."
— Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
```

---

</div>

## 📖 About Airlys

**Airlys** is a sovereign, from-scratch 64-bit operating system kernel designed and engineered by **Er. Adam Eehan** in pair programming with **Sovereign AI Aira 3 Ultra (Engine v3.6)**.

Unlike modern operating systems burdened by decades of legacy enterprise baggage, Airlys is built with pure architectural clarity:
- **Zero Standard Library Dependency:** 100% freestanding C and Assembly.
- **Dual Bootloader Compliance:** Native Multiboot 1 and Multiboot 2 headers. Boots out-of-the-box on direct hypervisors (QEMU), GRUB, and bare-metal PC hardware.
- **64-Bit Long Mode Transition:** Trampolines the CPU from legacy 32-bit Protected Mode directly into x86_64 Long Mode with 4-level paging (PML4, PDPT, Page Directory) identity-mapping the first 1GB of physical memory using 2MB huge pages.
- **Hardware-Direct Console:** Memory-mapped VGA text buffer driver (`0xB8000`) with color formatting, hardware cursor synchronization, horizontal tab parsing, and screen scrolling.
- **Sovereign Kernel Panic Subsystem:** Failsafe panic handler with CPU halt loop (`cli; hlt`).

---

## 🏛️ Kernel Architecture

```
+-------------------------------------------------------+
|                Airlys Ring 3 Userland                 |
|             (Native Shell & AiraLang Apps)            |
+-------------------------------------------------------+
                           |  syscall / sysret
+-------------------------------------------------------+
|                Airlys Kernel (Ring 0)                 |
|  +--------------------+  +-------------------------+  |
|  | Process Scheduler  |  | Virtual File System     |  |
|  +--------------------+  +-------------------------+  |
|  +--------------------+  +-------------------------+  |
|  | Paging & Heap      |  | IDT / Interrupts & PIC  |  |
|  +--------------------+  +-------------------------+  |
|  +--------------------+  +-------------------------+  |
|  | VGA Console Buffer |  | Global Descriptor Table |  |
|  +--------------------+  +-------------------------+  |
+-------------------------------------------------------+
                           |
+-------------------------------------------------------+
|          Bare-Metal Hardware / QEMU Virtual Machine   |
|                 (x86_64 Long Mode)                    |
+-------------------------------------------------------+
```

---

## 🚀 Building & Testing

### 🛠️ Prerequisites
- `clang` (LLVM C compiler with freestanding x86_64 cross-compilation support)
- `ld.lld` (LLVM linker)
- `make`

### 🔨 Build the Kernel Binary
```bash
cd Airlys
make clean
make
```

Upon compilation, the kernel binary is generated at:
```bash
bin/airlys.elf
```

### 💻 Running on QEMU / VirtualBox / PC
To run Airlys directly on QEMU:
```bash
qemu-system-x86_64 -kernel bin/airlys.elf
```

---

## 🗺️ Architectural Roadmap

- [x] **Phase 1: Genesis Boot & Console**
  - [x] Multiboot 1 & Multiboot 2 specification headers
  - [x] CPUID & Long Mode capability detection
  - [x] 4-level paging initialization (PML4 -> PDPT -> PD with 2MB huge pages)
  - [x] 64-bit Global Descriptor Table (GDT64)
  - [x] Hardware VGA text console driver (`0xB8000`)
  - [x] Failsafe Kernel Panic screen
  - [x] Clean zero-warning freestanding build system (`clang` + `ld.lld`)

- [x] **Phase 2: CPU Interrupts & Hardware Timing**
  - [x] Interrupt Descriptor Table (IDT64) & 256 gates (CPU exceptions 0..31)
  - [x] Dual 8259 PIC remapping (Master 32..39, Slave 40..47) with EOI handling
  - [x] Programmable Interval Timer (PIT 8254) running at 100 Hz (10ms preemptive ticks)
  - [x] PS/2 Hardware Keyboard driver with Scan Code Set 1 decoding
  - [x] CPU hardware interrupts enabled (`sti`) with live interactive keyboard typing!

- [x] **Phase 3: Sovereign Memory Subsystem**
  - [x] Physical Memory Manager (Bitmap Page Allocator tracking 1GB physical RAM & 262,144 frames)
  - [x] Virtual Memory Manager (Page fault `#PF` exception handler with CR2/CR3 register inspection)
  - [x] Sovereign Dynamic Kernel Heap (`kmalloc`, `kfree`, `kcalloc`, `krealloc` with 32MB dynamic pool & automated memory self-test)

- [ ] **Phase 4: Preemptive Multitasking**
  - [ ] Process Control Block (PCB) architecture
  - [ ] Timer-driven Round-Robin process scheduler
  - [ ] Ring 3 User-Mode execution transition

- [ ] **Phase 5: Virtual File System & Userland**
  - [ ] VFS abstraction layer
  - [ ] Initial RAMDisk (initrd / TarFS)
  - [ ] Interactive `airlys-sh` userland shell

- [ ] **Phase 6: Sovereign Network Stack & AiraLang Engine**
  - [ ] Gigabit Ethernet Driver (VirtIO-Net / Intel e1000)
  - [ ] Native Freestanding TCP/IP, UDP, ARP & ICMP network stack
  - [ ] Embedded AiraLang bytecode interpreter running natively in Ring 0 / Ring 3

- [ ] **Phase 7: Unified Sovereign Desktop (GUI), AiraStore & Standby AI**
  - [ ] High-Resolution VESA BIOS Extensions (VBE) & UEFI GOP Framebuffer Graphics Driver (1080p/4K)
  - [ ] Native Window Compositor, TrueType/Bitmap Font Engine & Cyber Eagle Dark Cyan UI
  - [ ] Essential Desktop Suite: `airlys-term`, `airlys-files`, `aira-code` Studio & Task Manager
  - [ ] **AiraStore**: Open-source sovereign app store with 4-tier anti-phishing sandbox & APK runtime
  - [ ] **Low-Power Neural Standby**: ACPI S0ix modern standby kernel scheduler for 2W 24/7 background AI inference

---

## 📜 Intellectual Property & Authorship

- **Creator & Lead Systems Architect:** [Er. Adam Eehan](https://github.com/adameehanairagroup) (Founder & CEO, Aira Group of Technology)
- **Co-Pilot & Engineering AI:** Sovereign AI **Aira 3 Ultra** (Engine v3.6)
- **Organization:** [Aira Group of Technology](https://github.com/airagroupoftechnology)
- **License:** Open Source under the MIT License.
