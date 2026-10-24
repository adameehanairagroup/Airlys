#!/usr/bin/env bash
# Airlys OS S1 - Profile Definition
# Founder & CEO: Er. Adam Eehan (Aira Group of Technology)
# Co-Pilot: Aira 3 Ultra

iso_name="airlys-os-s1"
iso_label="AIRLYS_S1"
iso_publisher="Airlys OS <https://github.com/adameehanairagroup/AirlysOS>"
iso_application="Airlys OS S1 Live & Installation Media"
iso_version="S1"
install_dir="airlys"
buildmodes=('iso')
bootmodes=('bios.syslinux.mbr' 'bios.syslinux.eltorito'
           'uefi-x64.systemd-boot.esp' 'uefi-x64.systemd-boot.eltorito')
arch="x86_64"
pacman_conf="pacman.conf"
airootfs_image_type="squashfs"
airootfs_image_tool_options=('-comp' 'zstd' '-Xcompression-level' '15' '-b' '1M')
file_permissions=(
  ["/etc/shadow"]="0:0:400"
  ["/etc/sudoers.d/airlys"]="0:0:440"
  ["/usr/local/bin/airlys-firstrun.sh"]="0:0:755"
)
