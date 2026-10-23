/*
 * gen_init_cpio.c - Generate cpio archive for initramfs
 * Ported & adapted for Airlys OS from Linux usr/gen_init_cpio.c
 * Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CPIO_MAGIC "070701"

static unsigned int c_ino = 721;

static void push_hdr(FILE *out, const char *name, unsigned int mode,
                     unsigned int uid, unsigned int gid, unsigned int nlink,
                     unsigned int mtime, unsigned int size) {
    char header[111];
    unsigned int namesize = (unsigned int)strlen(name) + 1;

    snprintf(header, sizeof(header),
             "%06s%08X%08X%08X%08X%08X%08X%08X%08X%08X%08X%08X%08X%08X",
             CPIO_MAGIC,
             c_ino++,
             mode,
             uid,
             gid,
             nlink,
             mtime,
             size,
             3, 1, 0, 0,
             namesize,
             0);

    fwrite(header, 110, 1, out);
    fwrite(name, namesize, 1, out);

    int pad = (4 - ((110 + namesize) % 4)) % 4;
    while (pad--) fputc(0, out);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Airlys Initramfs CPIO Generator\n");
        printf("Usage: %s <output.cpio> [file_list]\n", argv[0]);
        return 1;
    }

    FILE *out = fopen(argv[1], "wb");
    if (!out) {
        perror("fopen");
        return 1;
    }

    /* Default root dirs */
    push_hdr(out, "usr", 0040755, 0, 0, 2, 0, 0);
    push_hdr(out, "usr/bin", 0040755, 0, 0, 2, 0, 0);
    push_hdr(out, "usr/lib", 0040755, 0, 0, 2, 0, 0);
    push_hdr(out, "etc", 0040755, 0, 0, 2, 0, 0);
    push_hdr(out, "bin", 0040755, 0, 0, 2, 0, 0);

    /* Trailer */
    push_hdr(out, "TRAILER!!!", 0, 0, 0, 1, 0, 0);

    fclose(out);
    return 0;
}
