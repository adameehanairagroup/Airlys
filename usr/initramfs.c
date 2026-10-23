#include <airlys/initramfs.h>
#include <airlys/vfs.h>
#include <airlys/ramfs.h>
#include <airlys/string.h>
#include <airlys/printk.h>

static initramfs_status_t g_initramfs_status = {
    .total_files = 0,
    .total_dirs = 0,
    .total_bytes = 0,
    .initialized = false
};

static uint32_t parse_hex8(const char* hex) {
    uint32_t val = 0;
    for (int i = 0; i < 8; i++) {
        char c = hex[i];
        val <<= 4;
        if (c >= '0' && c <= '9') val |= (c - '0');
        else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
    }
    return val;
}

int initramfs_unpack(const void* data, size_t size) {
    if (!data || size < sizeof(cpio_header_t)) {
        return -1;
    }

    const uint8_t* ptr = (const uint8_t*)data;
    const uint8_t* end = ptr + size;
    vfs_node_t* root = vfs_get_root();
    if (!root) return -2;

    while (ptr + sizeof(cpio_header_t) <= end) {
        const cpio_header_t* hdr = (const cpio_header_t*)ptr;
        if (kmemcmp(hdr->c_magic, CPIO_NEWC_MAGIC, 6) != 0) {
            break;
        }

        uint32_t namesize = parse_hex8(hdr->c_namesize);
        uint32_t filesize = parse_hex8(hdr->c_filesize);
        uint32_t mode = parse_hex8(hdr->c_mode);

        ptr += sizeof(cpio_header_t);
        if (ptr + namesize > end) break;

        const char* filename = (const char*)ptr;
        if (kstrcmp(filename, CPIO_TRAILER) == 0) {
            break; /* End of archive */
        }

        /* Pad to 4-byte boundary */
        ptr += namesize;
        size_t pad = (4 - ((sizeof(cpio_header_t) + namesize) % 4)) % 4;
        ptr += pad;

        if (ptr + filesize > end) break;

        /* Is Directory */
        if ((mode & 0170000) == 0040000) {
            /* Create directory in VFS */
            if (kstrcmp(filename, ".") != 0) {
                ramfs_create_dir(root, filename);
                g_initramfs_status.total_dirs++;
            }
        }
        /* Is Regular File */
        else if ((mode & 0170000) == 0100000) {
            /* For now, text/small payload copy */
            char temp_content[512];
            size_t copy_len = filesize < 511 ? filesize : 511;
            kmemcpy(temp_content, ptr, copy_len);
            temp_content[copy_len] = '\0';

            ramfs_create_file(root, filename, temp_content);
            g_initramfs_status.total_files++;
            g_initramfs_status.total_bytes += filesize;
        }

        ptr += filesize;
        pad = (4 - (filesize % 4)) % 4;
        ptr += pad;
    }

    g_initramfs_status.initialized = true;
    return 0;
}

void initramfs_init(void) {
    vfs_node_t* root = vfs_get_root();
    if (!root) return;

    /* Build canonical Linux system tree in VFS */
    vfs_node_t* usr_dir = vfs_finddir(root, "usr");
    if (!usr_dir) {
        usr_dir = ramfs_create_dir(root, "usr");
        g_initramfs_status.total_dirs++;
    }
    if (usr_dir) {
        ramfs_create_dir(usr_dir, "bin");
        ramfs_create_dir(usr_dir, "lib");
        ramfs_create_dir(usr_dir, "share");
        ramfs_create_dir(usr_dir, "include");
        g_initramfs_status.total_dirs += 4;
    }

    vfs_node_t* sbin_dir = vfs_finddir(root, "sbin");
    if (!sbin_dir) {
        ramfs_create_dir(root, "sbin");
        g_initramfs_status.total_dirs++;
    }

    vfs_node_t* var_dir = vfs_finddir(root, "var");
    if (!var_dir) {
        var_dir = ramfs_create_dir(root, "var");
        ramfs_create_dir(var_dir, "log");
        ramfs_create_dir(var_dir, "run");
        g_initramfs_status.total_dirs += 3;
    }

    vfs_node_t* etc_dir = vfs_finddir(root, "etc");
    if (etc_dir) {
        ramfs_create_file(etc_dir, "hostname", "airlys-sovereign\n");
        ramfs_create_file(etc_dir, "issue", "Airlys GNU/Linux Sovereign LTS (x86_64) \\n \\l\n");
        ramfs_create_file(etc_dir, "passwd", "root:x:0:0:root:/root:/bin/sh\nuser:x:1000:1000:Adam Eehan:/home/user:/bin/sh\n");
        ramfs_create_file(etc_dir, "group", "root:x:0:\nwheel:x:10:root,user\nusers:x:100:user\n");
        ramfs_create_file(etc_dir, "environment", "PATH=/usr/bin:/bin:/usr/sbin:/sbin\nEDITOR=nano\nLANG=en_US.UTF-8\n");
        g_initramfs_status.total_files += 5;
    }

    g_initramfs_status.initialized = true;
    printk(KERN_INFO "initramfs: Populated Linux canonical rootfs hierarchy (/usr, /etc, /var, /sbin)\n");
}

initramfs_status_t initramfs_get_status(void) {
    return g_initramfs_status;
}
