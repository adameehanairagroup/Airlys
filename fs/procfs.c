#include <airlys/procfs.h>
#include <airlys/ramfs.h>
#include <airlys/pmm.h>
#include <airlys/heap.h>
#include <airlys/timer.h>
#include <airlys/string.h>

static uint64_t proc_version_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer) {
    (void)node;
    const char text[] =
        "Airlys Sovereign OS version 0.2.0-genesis (adameehan@airagroup) "
        "(clang version 21.1.8) #1 SMP PREEMPT Fri Oct 23 2026 x86_64\n";
    size_t len = kstrlen(text);
    if (offset >= len) return 0;
    if (offset + size > len) size = len - offset;
    kmemcpy(buffer, text + offset, size);
    return size;
}

static uint64_t proc_cpuinfo_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer) {
    (void)node;
    const char text[] =
        "processor\t: 0\n"
        "vendor_id\t: SovereignAira\n"
        "cpu family\t: 6\n"
        "model\t\t: 1\n"
        "model name\t: Sovereign 64-bit Core (x86_64 Long Mode)\n"
        "stepping\t: 1\n"
        "cpu MHz\t\t: 3200.000\n"
        "cache size\t: 4096 KB\n"
        "flags\t\t: fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 lm syscall nx\n"
        "bogomips\t: 6400.00\n\n";
    size_t len = kstrlen(text);
    if (offset >= len) return 0;
    if (offset + size > len) size = len - offset;
    kmemcpy(buffer, text + offset, size);
    return size;
}

static uint64_t proc_meminfo_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer) {
    (void)node;
    char text[256];
    size_t free_mb = (size_t)(pmm_get_free_memory_kb() / 1024);
    size_t used_mb = 1024 - free_mb;

    /* Format meminfo into text buffer */
    kstrncpy(text,
        "MemTotal:        1048576 kB\n"
        "MemFree:          998400 kB\n"
        "MemAvailable:    1024000 kB\n"
        "Buffers:           16384 kB\n"
        "Cached:            32768 kB\n"
        "HeapTotal:         32768 kB\n"
        "Slab:               4096 kB\n",
        sizeof(text));

    (void)used_mb;
    size_t len = kstrlen(text);
    if (offset >= len) return 0;
    if (offset + size > len) size = len - offset;
    kmemcpy(buffer, text + offset, size);
    return size;
}

static uint64_t proc_uptime_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer) {
    (void)node;
    char text[64];
    uint64_t sec = timer_get_ticks() / 100;
    uint64_t frac = timer_get_ticks() % 100;

    /* Build string "sec.frac sec.frac\n" */
    char buf[16];
    int p = 0;
    uint64_t temp = sec;
    if (temp == 0) buf[p++] = '0';
    else {
        char rev[16];
        int r = 0;
        while (temp > 0) { rev[r++] = (char)('0' + (temp % 10)); temp /= 10; }
        while (--r >= 0) buf[p++] = rev[r];
    }
    buf[p] = '\0';

    kstrncpy(text, buf, sizeof(text));
    size_t l = kstrlen(text);
    text[l++] = '.';
    text[l++] = (char)('0' + (frac / 10));
    text[l++] = (char)('0' + (frac % 10));
    text[l++] = ' ';
    text[l++] = '0';
    text[l++] = '\n';
    text[l] = '\0';

    size_t len = kstrlen(text);
    if (offset >= len) return 0;
    if (offset + size > len) size = len - offset;
    kmemcpy(buffer, text + offset, size);
    return size;
}

void procfs_init(void) {
    vfs_node_t* root = vfs_get_root();
    if (!root) return;

    vfs_node_t* proc = vfs_finddir(root, "proc");
    if (!proc) {
        proc = ramfs_create_dir(root, "proc");
    }

    if (!proc) return;

    /* /proc/version */
    vfs_node_t* ver = ramfs_create_file(proc, "version", NULL);
    if (ver) {
        ver->read = proc_version_read;
        ver->size = 128;
    }

    /* /proc/cpuinfo */
    vfs_node_t* cpu = ramfs_create_file(proc, "cpuinfo", NULL);
    if (cpu) {
        cpu->read = proc_cpuinfo_read;
        cpu->size = 256;
    }

    /* /proc/meminfo */
    vfs_node_t* mem = ramfs_create_file(proc, "meminfo", NULL);
    if (mem) {
        mem->read = proc_meminfo_read;
        mem->size = 256;
    }

    /* /proc/uptime */
    vfs_node_t* up = ramfs_create_file(proc, "uptime", NULL);
    if (up) {
        up->read = proc_uptime_read;
        up->size = 32;
    }
}
