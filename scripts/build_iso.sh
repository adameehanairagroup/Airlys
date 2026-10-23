#!/usr/bin/env bash
# Airlys Sovereign OS - Bootable ISO Generator
# Engineered by Er. Adam Eehan & Sovereign AI Aira

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

KERNEL="${ROOT_DIR}/bin/airlys.elf"
ISO_DIR="${ROOT_DIR}/build/isodir"
OUTPUT_ISO="${ROOT_DIR}/bin/airlys.iso"

if [ ! -f "${KERNEL}" ]; then
    echo " [*] Compiling Airlys kernel..."
    make -C "${ROOT_DIR}"
fi

echo " [*] Preparing GRUB2 bootable structure..."
mkdir -p "${ISO_DIR}/boot/grub"
cp "${KERNEL}" "${ISO_DIR}/boot/airlys.elf"

cat << 'EOF' > "${ISO_DIR}/boot/grub/grub.cfg"
insmod all_video
insmod efi_gop
insmod efi_uga
insmod font

set gfxmode=1920x1080x32,1024x768x32,auto
set gfxpayload=1920x1080x32,1024x768x32,auto

set timeout=5
set default=0

menuentry "Airlys Sovereign OS (Acer Nitro Bare Metal & UEFI GOP)" {
    insmod all_video
    insmod efi_gop
    set gfxpayload=1920x1080x32,1024x768x32,auto
    multiboot2 /boot/airlys.elf
    boot
}

menuentry "Airlys Sovereign OS (1024x768 TrueColor Framebuffer)" {
    insmod all_video
    set gfxpayload=1024x768x32,auto
    multiboot2 /boot/airlys.elf
    boot
}

menuentry "Airlys Sovereign OS (1920x1080 Native FHD Framebuffer)" {
    insmod all_video
    insmod efi_gop
    set gfxpayload=1920x1080x32,auto
    multiboot2 /boot/airlys.elf
    boot
}

menuentry "Airlys Sovereign OS (Fallback Multiboot 1)" {
    insmod all_video
    set gfxpayload=auto
    multiboot /boot/airlys.elf
    boot
}
EOF

if command -v grub-mkrescue &> /dev/null; then
    echo " [*] Generating ${OUTPUT_ISO}..."
    grub-mkrescue -o "${OUTPUT_ISO}" "${ISO_DIR}"
    echo " [OK] Bootable ISO generated successfully: ${OUTPUT_ISO}"
else
    echo " [!] grub-mkrescue not installed in local environment."
    echo "     ISO staging directory ready at: ${ISO_DIR}"
fi
