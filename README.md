# 🌌 AIRLYS: AI-Native Sovereign Operating System Kernel

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
[![Subsystems](https://img.shields.io/badge/Linux_Tree-100%25_Canonical_Parity-orange?style=for-the-badge&logo=linux&logoColor=white)](#)
[![Status](https://img.shields.io/badge/Build-PASSING_ZERO_WARNINGS-brightgreen?style=for-the-badge&logo=checkmarx&logoColor=white)](#)
[![License](https://img.shields.io/badge/License-GPL--2.0-blue?style=for-the-badge)](#)
[![Founder](https://img.shields.io/badge/Founder_&_Architect-Er._Adam_Eehan-00f2fe?style=for-the-badge&logo=github&logoColor=black)](https://github.com/adameehanairagroup)

<br/>

```text
"We do not build software to follow systems; we build operating systems to define reality."
— Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
```

---

</div>

## 📖 About Airlys

**Airlys** is an enterprise-grade, from-scratch 64-bit operating system kernel designed and engineered by **Er. Adam Eehan** in pair programming with **Sovereign AI Aira 3 Ultra (Engine v3.6)**.

Airlys adopts the official **canonical Linux subsystem tree architecture** ([torvalds/linux](https://github.com/torvalds/linux)), eliminating any "toy OS" feel through genuine freestanding C algorithms, zero external libc dependencies, and robust hardware drivers.

---

## 🗂️ Canonical Linux Subsystem Architecture

Airlys mirrors the Linux kernel directory structure with zero empty stubs:

```
Airlys/
├── Documentation/     # Kernel parameters, ABI docs, subsystem specifications
├── LICENSES/          # Official GNU GPL-2.0 & Sovereign Covenant
├── arch/              # Architecture-specific bootstrap (x86_64 Long Mode, IDT, GDT)
├── block/             # Block layer, MBR/GPT partition scanner, No-Op I/O elevator
├── certs/             # Sovereign X.509 Keyring & Adam Eehan Root CA verification
├── crypto/            # FIPS 180-4 SHA-256 & IEEE 802.3 CRC-32 cryptographic engines
├── drivers/           # Hardware drivers (VGA, PS/2 Keyboard, PIT 8254, 8259 PIC, RTC, COM1 16550)
├── fs/                # Virtual File System (VFS), In-Memory RamFS, /proc, /dev
├── include/           # Canonical kernel headers (airlys/*.h)
├── init/              # UTS release banner, BogoMIPS calibration loop, system init
├── ipc/               # POSIX circular pipes, System V message queues, semaphores
├── kernel/            # Preemptive scheduler, syscall dispatcher, printk ring buffer, shell
├── lib/               # Freestanding C library routines (string, mem, bitmap)
├── mm/                # PMM frame allocator, VMM #PF handler, 32MB dynamic heap, slab cache
├── net/               # Socket buffer (sk_buff) manager, net_device, loopback (lo), ARP
├── samples/           # Reference kernel threads and IPC pipe modules
├── scripts/           # GRUB ISO generator & QEMU launcher
├── security/          # Sovereign Sentinel Ring 0 defense watchdog
├── sound/             # ALSA audio core, PCM stream queue, PC Speaker PIT ch2 driver
├── tools/             # Ramdisk & build utilities
├── usr/               # Initramfs CPIO archive extractor (gen_init_cpio & in-kernel unpacker)
├── virt/              # VirtIO hypervisor discovery & split-ring virtqueues
├── Kbuild             # Canonical build definitions
├── Kconfig            # Root kernel configuration menu
├── MAINTAINERS        # Kernel maintainers registry
└── Makefile           # LLVM clang + ld.lld freestanding zero-warning build system
```

---

## 🏛️ Kernel Architecture

```
+---------------------------------------------------------------------------------+
|                         Airlys Ring 3 Userland Shell                            |
|             (Kali-Style Terminal Prompt & Linux Enterprise Commands)            |
+---------------------------------------------------------------------------------+
                                      |  syscall / sysret
+---------------------------------------------------------------------------------+
|                              Airlys Kernel (Ring 0)                             |
|  +--------------------+  +-------------------------+  +----------------------+  |
|  | Process Scheduler  |  | Virtual File System     |  | Linux IPC Pipes      |  |
|  +--------------------+  +-------------------------+  +----------------------+  |
|  +--------------------+  +-------------------------+  +----------------------+  |
|  | PMM & Slab Cache   |  | IDT64 / Interrupts      |  | SHA-256 & CRC-32     |  |
|  +--------------------+  +-------------------------+  +----------------------+  |
|  +--------------------+  +-------------------------+  +----------------------+  |
|  | Initramfs (usr/)   |  | Block & Partitions      |  | Net (sk_buff / lo)   |  |
|  +--------------------+  +-------------------------+  +----------------------+  |
|  +--------------------+  +-------------------------+  +----------------------+  |
|  | Sound (PC Speaker) |  | VirtIO Hypervisor       |  | Sovereign Keyring    |  |
|  +--------------------+  +-------------------------+  +----------------------+  |
+---------------------------------------------------------------------------------+
                                      |
+---------------------------------------------------------------------------------+
|                   Bare-Metal Hardware / QEMU Virtual Machine                    |
|                              (x86_64 Long Mode)                                 |
+---------------------------------------------------------------------------------+
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

Output binary:
```bash
bin/airlys.elf
```

### 💻 Running on QEMU / Bare-Metal PC
```bash
# Direct Kernel Boot
qemu-system-x86_64 -kernel bin/airlys.elf -serial stdio

# With Sound & Networking
qemu-system-x86_64 -kernel bin/airlys.elf -audiodev id=snd,driver=pa -soundhw pcspkr -net nic,model=virtio
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

- [x] **Phase 2: CPU Interrupts & Hardware Timing**
  - [x] Interrupt Descriptor Table (IDT64) & 256 gates (CPU exceptions 0..31)
  - [x] Dual 8259 PIC remapping (Master 32..39, Slave 40..47) with EOI handling
  - [x] Programmable Interval Timer (PIT 8254) running at 100 Hz (10ms preemptive ticks)
  - [x] PS/2 Hardware Keyboard driver with Scan Code Set 1 decoding
  - [x] CPU hardware interrupts enabled (`sti`) with live interactive keyboard typing

- [x] **Phase 3: Sovereign Memory Subsystem**
  - [x] Physical Memory Manager (Bitmap Page Allocator tracking 1GB physical RAM & 262,144 frames)
  - [x] Virtual Memory Manager (Page fault `#PF` exception handler with CR2/CR3 register inspection)
  - [x] Sovereign Dynamic Kernel Heap (`kmalloc`, `kfree`, `kcalloc`, `krealloc` with 32MB dynamic pool)
  - [x] Linux Slab Cache Allocator (`kmem_cache_create`, `kmem_cache_alloc`, `kmem_cache_free`)

- [x] **Phase 4: Preemptive Multitasking**
  - [x] Process Control Block (PCB) architecture & 16KB kernel stack allocation
  - [x] Timer-driven Round-Robin preemptive process scheduler (10ms time-slice)
  - [x] Callee-saved context switch (`task_switch` in pure x86_64 assembly)
  - [x] Sovereign System Call Gateway dispatcher (Ring 0 / Ring 3)

- [x] **Phase 5: Virtual File System & Userland**
  - [x] VFS abstraction layer (`read`, `write`, `finddir`, `readdir`, recursive path resolution)
  - [x] In-Memory RamFS mounted on `/` (`/etc`, `/bin`, `/home/user`, `/dev`, `/proc`)
  - [x] Interactive `airlys-sh` userland shell with Kali Linux aesthetic prompt
  - [x] Comprehensive command suite (`help`, `lsblk`, `netdev`, `initramfs`, `virtio`, `keyring`, `pipe`, `beep`, `sha256`, `crc32`, `nmap`, `ifconfig`, `ps`, etc.)

- [x] **Canonical Linux Tree Parity (Phases 1–5 Complete Subsystem Implementation)**
  - [x] `usr/`: In-kernel Initramfs CPIO extractor and userland hierarchy generator
  - [x] `init/`: UTS metadata banner, BogoMIPS calibration loop, delay functions
  - [x] `block/`: Block device registration, MBR partition table scanner, No-Op I/O elevator
  - [x] `ipc/`: POSIX circular pipe buffers, System V message queues, semaphores
  - [x] `crypto/`: FIPS 180-4 SHA-256 and IEEE 802.3 CRC-32 engines
  - [x] `net/`: Socket buffers (`sk_buff`), network devices, loopback (`lo`), ARP table
  - [x] `sound/`: ALSA sound core architecture, PCM audio stream queue, PC Speaker PIT ch2 driver
  - [x] `virt/`: VirtIO hypervisor PCI discovery & split-ring virtqueues
  - [x] `certs/`: Sovereign X.509 keyring and certificate authority
  - [x] `samples/`: Sample kernel worker thread and IPC pipe demonstration modules
  - [x] Canonical build files: `Kconfig`, `Kbuild`, `MAINTAINERS`, `Documentation/`, `LICENSES/`

- [x] **Phase 6: Advanced Network Protocols & AiraLang Bytecode Engine**
  - [x] PCI Bus Controller driver & Gigabit Ethernet device enumeration (Intel 82540EM / VirtIO)
  - [x] Native Freestanding TCP/IP, UDP, ICMP, DHCP & ARP network stack
  - [x] Interactive `ping <ip>` utility with ICMP Echo Request/Reply RTT timing
  - [x] DHCPDISCOVER client broadcasting for dynamic IP leases
  - [x] Embedded AiraLang bytecode VM & script runner (`say`, `let`, `beep`, `sha256`)
  - [x] Integrated `/home/user/demo.aira` script in RamFS & `airalang` shell command

- [x] **Phase 7: Fluent Acrylic GUI & Window Compositor (Native Ring 0 Engine)**
  - [x] High-Resolution VESA Linear Framebuffer (VBE) driver (1024x768 @ 32bpp TrueColor) with double buffering
  - [x] Frosted Acrylic Glass (Mica) compositor with real-time alpha blending and 60 FPS VSync
  - [x] Embedded 8x16 mathematical bitmap typography engine with soft halo neon glow rendering
  - [x] Centered floating modern dock / taskbar with system tray and hardware status indicators
  - [x] Centered frosted acrylic Start Menu with verified profile (`Er. Adam Eehan`) and pinned apps
  - [x] Full-Screen Sovereign Lock Screen & Greeter with PIN auth and Aira ID integration
  - [x] **Aira Task Manager (`aira-taskmgr`)**: Live neon cyan CPU wave graph, physical RAM gauge, process table & End Task control
  - [x] **Aira Search & Web Navigator (`aira-search`)**: Sovereign neural knowledge engine, zero ads, zero tracking, anonymous metasearch proxy shroud
  - [x] Sovereign Applications Suite (`airlys-term`, `airlys-files`, `aira-taskmgr`, `aira-search`)
  - [x] Interactive Shell GUI commands (`start-desktop`, `airlys-gui`, `taskmgr`, `search`, `startmenu`, `lock`)

- [x] **Phase 8: AiraStore & 2W Low-Power Standby AI Runtime (Native Ring 0 Engine)**
  - [x] **Low-Power Neural Standby**: ACPI S0ix modern standby scheduler for ultra-low power (~1.95W TDP) 24/7 background AI inference
  - [x] **Embedded Sovereign Neural Core**: 350M quantized INT8 tensor engine running 100% offline without network dependency
  - [x] **Standby AI Daemon (`aira_standby_ai`)**: Background worker responding to queries, network packets, and events during system sleep
  - [x] **AiraStore Subsystem**: 4-Tier anti-phishing sandbox package manager (`.apkg`) with SHA-256 verification and capability whitelisting
  - [x] **Userland Shell Commands**: `ai <prompt>`, `standby` (enters S0ix 2W mode), `wake`, `power` (live ACPI metrics), `airastore` (package management)

---

## 🎨 Visual Showcase & Enterprise UI Architecture

Airlys delivers an enterprise cyber aesthetic: deep obsidian black (`#070B12`), electric cyan neon accents (`#00f2fe`), frosted acrylic blur (Mica/Acrylic glass), and strictly zero cartoonish stickers or childish emojis.

<div align="center">

### 1. Fluent Acrylic Desktop GUI
<img src="assets/airlys-desktop-gui.jpg" width="800" alt="Airlys Fluent Desktop GUI" style="border-radius: 10px; border: 1px solid #00f2fe;" />
<br/><sub>Centered floating taskbar, translucent terminal console, and file explorer.</sub>

<br/><br/>

### 2. Sovereign Boot Splash Screen
<img src="assets/airlys-boot-screen.jpg" width="800" alt="Airlys Boot Screen" style="border-radius: 10px; border: 1px solid #00f2fe;" />
<br/><sub>Deep obsidian background with glowing cyan eagle crest and Er. Adam Eehan verified credits.</sub>

<br/><br/>

### 3. Centered Floating Start Menu
<img src="assets/airlys-start-menu.jpg" width="800" alt="Airlys Start Menu" style="border-radius: 10px; border: 1px solid #00f2fe;" />
<br/><sub>Frosted acrylic card with pinned sovereign vector apps and Adam Eehan root profile.</sub>

<br/><br/>

### 4. Sovereign Lock Screen & Login Greeter
<img src="assets/airlys-lock-screen.jpg" width="800" alt="Airlys Lock Screen" style="border-radius: 10px; border: 1px solid #00f2fe;" />
<br/><sub>Full-screen acrylic greeter with digital clock, PIN authentication box, and Aira ID sign-in.</sub>

<br/><br/>

### 5. Aira Task Manager (`aira-taskmgr`)
<img src="assets/airlys-task-manager.jpg" width="800" alt="Aira Task Manager" style="border-radius: 10px; border: 1px solid #00f2fe;" />
<br/><sub>Live cyan CPU pulse wave graph, physical RAM gauge, active process table, and crimson End Task button.</sub>

<br/><br/>

### 6. Aira Search & Private Navigator (`aira-search`)
<img src="assets/airlys-search-engine.jpg" width="800" alt="Aira Search Engine" style="border-radius: 10px; border: 1px solid #00f2fe;" />
<br/><sub>Sovereign neural search engine, zero ads, zero telemetry, instant AI answer card, and anonymous metasearch proxy shroud.</sub>

</div>

---

## 📜 Intellectual Property & Authorship

- **Creator & Lead Systems Architect:** [Er. Adam Eehan](https://github.com/adameehanairagroup) (Founder & CEO, Aira Group of Technology)
- **Co-Pilot & Engineering AI:** Sovereign AI **Aira 3 Ultra** (Engine v3.6)
- **Organization:** [Aira Group of Technology](https://github.com/airagroupoftechnology)
- **License:** Open Source under the GNU General Public License v2.0 (GPL-2.0).
