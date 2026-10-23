#!/usr/bin/env bash
# Airlys Sovereign OS - QEMU Kernel Runner
# Aira Group of Technology

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

KERNEL="${ROOT_DIR}/bin/airlys.elf"

if [ ! -f "${KERNEL}" ]; then
    echo " [*] Compiling Airlys kernel..."
    make -C "${ROOT_DIR}"
fi

echo " [🚀] Launching Airlys Sovereign OS in QEMU (x86_64, 1GB RAM)..."
qemu-system-x86_64 \
    -kernel "${KERNEL}" \
    -m 1G \
    -serial stdio \
    -vga std \
    -no-reboot
