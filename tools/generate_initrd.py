#!/usr/bin/env python3
"""
Airlys Sovereign OS - RAMDisk / Initrd Archive Generator
Engineered for Aira Group of Technology
"""

import sys
import os
import struct

def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <input_directory> <output_initrd.img>")
        return 1

    input_dir = sys.argv[1]
    output_file = sys.argv[2]

    print(f"[*] Packing {input_dir} into Airlys RAMDisk: {output_file}")
    # Header format: AIRA_RD0 (magic 8 bytes), entry count (4 bytes)
    entries = []
    for root, dirs, files in os.walk(input_dir):
        for f in files:
            full_path = os.path.join(root, f)
            rel_path = os.path.relpath(full_path, input_dir).replace('\\', '/')
            with open(full_path, 'rb') as fp:
                data = fp.read()
            entries.append((rel_path, data))

    with open(output_file, 'wb') as out:
        out.write(b"AIRA_RD0")
        out.write(struct.pack("<I", len(entries)))
        for path, data in entries:
            encoded_path = path.encode('utf-8')[:64].ljust(64, b'\x00')
            out.write(encoded_path)
            out.write(struct.pack("<I", len(data)))
            out.write(data)

    print(f"[OK] Packed {len(entries)} files into {output_file}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
