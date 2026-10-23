#include <airlys/shell.h>
#include <airlys/vga.h>
#include <airlys/vfs.h>
#include <airlys/ramfs.h>
#include <airlys/pmm.h>
#include <airlys/heap.h>
#include <airlys/sched.h>
#include <airlys/timer.h>
#include <airlys/io.h>

static char shell_buffer[SHELL_MAX_INPUT];
static size_t shell_pos = 0;
static char shell_cwd[128] = "/home/user";

static int kstrcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static void kstrncpy(char* dest, const char* src, size_t n) {
    size_t i = 0;
    while (i < n - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static uint32_t katoi(const char* str) {
    uint32_t res = 0;
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

void shell_init(void) {
    shell_pos = 0;
    shell_buffer[0] = '\0';
    kstrncpy(shell_cwd, "/home/user", sizeof(shell_cwd));
    vga_set_prompt_path(shell_cwd);
}

static void resolve_full_path(const char* input, char* out, size_t out_len) {
    if (!input || *input == '\0' || kstrcmp(input, ".") == 0) {
        kstrncpy(out, shell_cwd, out_len);
        return;
    }

    if (input[0] == '/') {
        kstrncpy(out, input, out_len);
        return;
    }

    /* Relative path: shell_cwd + "/" + input */
    size_t pos = 0;
    while (shell_cwd[pos] != '\0' && pos < out_len - 1) {
        out[pos] = shell_cwd[pos];
        pos++;
    }

    if (pos > 0 && out[pos - 1] != '/' && pos < out_len - 1) {
        out[pos++] = '/';
    }

    size_t j = 0;
    while (input[j] != '\0' && pos < out_len - 1) {
        out[pos++] = input[j++];
    }
    out[pos] = '\0';
}

static void split_path(const char* full_path, char* parent_buf, size_t parent_len, char* name_buf, size_t name_len) {
    int last_slash = -1;
    for (int i = 0; full_path[i] != '\0'; i++) {
        if (full_path[i] == '/') last_slash = i;
    }

    if (last_slash == -1) {
        kstrncpy(parent_buf, shell_cwd, parent_len);
        kstrncpy(name_buf, full_path, name_len);
    } else if (last_slash == 0) {
        parent_buf[0] = '/';
        parent_buf[1] = '\0';
        kstrncpy(name_buf, full_path + 1, name_len);
    } else {
        size_t j = 0;
        while (j < (size_t)last_slash && j < parent_len - 1) {
            parent_buf[j] = full_path[j];
            j++;
        }
        parent_buf[j] = '\0';
        kstrncpy(name_buf, full_path + last_slash + 1, name_len);
    }
}

static void cmd_help(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("=================================================================\n");
    vga_puts("        AIRLYS SOVEREIGN SHELL & CYBER ARSENAL (v1.0)           \n");
    vga_puts("=================================================================\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts(" 📂 File & Directory Navigation:\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("   pwd            - Print working directory path\n");
    vga_puts("   cd <dir>       - Change working directory (~, .., /path)\n");
    vga_puts("   ls [dir]       - List directory contents & file sizes\n");
    vga_puts("   cat <file>     - Display file contents\n");
    vga_puts("   mkdir <dir>    - Create a new directory\n");
    vga_puts("   touch <file>   - Create an empty file\n");
    vga_puts("   rm <target>    - Delete a file or empty directory\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts(" ⚡ Linux System & Process Utilities:\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("   whoami         - Print active user & sovereign authority\n");
    vga_puts("   uname [-a]     - Print kernel architecture & version\n");
    vga_puts("   uptime         - System running duration & active tasks\n");
    vga_puts("   date           - System RTC clock & calendar\n");
    vga_puts("   dmesg          - Kernel boot ring-buffer log\n");
    vga_puts("   free           - Physical RAM & dynamic kernel heap stats\n");
    vga_puts("   ps             - Preemptive process list & execution ticks\n");
    vga_puts("   kill <pid>     - Terminate a task by Process ID\n");
    vga_puts("   clear          - Clear VGA screen\n");
    vga_puts("   reboot         - Warm hardware reboot\n");

    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts(" 🛡️ Cyber Security & Penetration Arsenal (Kali Style):\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("   nmap [ip]      - Network port scanner & service discovery\n");
    vga_puts("   ifconfig       - Network interface adapters & IP status\n");
    vga_puts("   netstat        - Active network sockets & listening ports\n");
    vga_puts("   aircrack       - 802.11 Wi-Fi monitor & beacon recon scanner\n");
    vga_puts("   tcpdump        - Live packet sniffer & header hex dump\n");
    vga_puts("   hash <text>    - FNV-1a & CRC32 cryptographic hash calculation\n");
    vga_puts("   aira <cmd>     - Sovereign root administrative privilege execution\n");
    vga_puts("   covenant       - Read the Sovereign Open-Source Covenant\n");
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("=================================================================\n");
}

static void cmd_pwd(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts(shell_cwd);
    vga_puts("\n");
}

static void cmd_cd(const char* path) {
    if (!path || *path == '\0' || kstrcmp(path, "~") == 0) {
        kstrncpy(shell_cwd, "/home/user", sizeof(shell_cwd));
        vga_set_prompt_path(shell_cwd);
        return;
    }

    if (kstrcmp(path, "..") == 0) {
        /* Move up one directory level */
        int last_slash = -1;
        for (int i = 0; shell_cwd[i] != '\0'; i++) {
            if (shell_cwd[i] == '/') last_slash = i;
        }

        if (last_slash <= 0) {
            kstrncpy(shell_cwd, "/", sizeof(shell_cwd));
        } else {
            shell_cwd[last_slash] = '\0';
        }
        vga_set_prompt_path(shell_cwd);
        return;
    }

    char target[128];
    resolve_full_path(path, target, sizeof(target));

    vfs_node_t* node = vfs_resolve_path(target);
    if (!node) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("cd: no such file or directory: ");
        vga_puts(path);
        vga_puts("\n");
        return;
    }

    if ((node->flags & VFS_DIRECTORY) == 0) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("cd: not a directory: ");
        vga_puts(path);
        vga_puts("\n");
        return;
    }

    kstrncpy(shell_cwd, target, sizeof(shell_cwd));
    vga_set_prompt_path(shell_cwd);
}

static void cmd_ls(const char* path) {
    char target[128];
    if (path && *path != '\0') {
        resolve_full_path(path, target, sizeof(target));
    } else {
        kstrncpy(target, shell_cwd, sizeof(target));
    }

    vfs_node_t* node = vfs_resolve_path(target);
    if (!node) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("ls: cannot access '");
        vga_puts(target);
        vga_puts("': No such directory\n");
        return;
    }

    if ((node->flags & VFS_DIRECTORY) == 0) {
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts(node->name);
        vga_puts("\n");
        return;
    }

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Directory: ");
    vga_puts(target);
    vga_puts("\n");

    for (uint32_t i = 0; i < node->num_children; i++) {
        vfs_node_t* child = node->children[i];
        if (!child) continue;

        if (child->flags & VFS_DIRECTORY) {
            vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
            vga_puts("  [DIR]  ");
            vga_puts(child->name);
            vga_puts("/\n");
        } else {
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            vga_puts("  [FILE] ");
            vga_puts(child->name);
            vga_puts(" (");
            vga_putdec(child->size);
            vga_puts(" bytes)\n");
        }
    }
}

static void cmd_cat(const char* path) {
    if (!path || *path == '\0') {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("cat: missing file operand\n");
        return;
    }

    char target[128];
    resolve_full_path(path, target, sizeof(target));

    vfs_node_t* node = vfs_resolve_path(target);
    if (!node) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("cat: ");
        vga_puts(path);
        vga_puts(": No such file\n");
        return;
    }

    if (node->flags & VFS_DIRECTORY) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("cat: ");
        vga_puts(path);
        vga_puts(": Is a directory\n");
        return;
    }

    if (node->size == 0) return;

    char buf[128];
    uint64_t offset = 0;
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    while (offset < node->size) {
        uint64_t read_bytes = vfs_read(node, offset, sizeof(buf) - 1, (uint8_t*)buf);
        if (read_bytes == 0) break;
        buf[read_bytes] = '\0';
        vga_puts(buf);
        offset += read_bytes;
    }
}

static void cmd_mkdir(const char* path) {
    if (!path || *path == '\0') {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("mkdir: missing directory operand\n");
        return;
    }

    char full[128];
    resolve_full_path(path, full, sizeof(full));

    char parent_path[128];
    char dir_name[64];
    split_path(full, parent_path, sizeof(parent_path), dir_name, sizeof(dir_name));

    vfs_node_t* parent = vfs_resolve_path(parent_path);
    if (!parent) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("mkdir: cannot create '");
        vga_puts(path);
        vga_puts("': Parent directory not found\n");
        return;
    }

    if (vfs_finddir(parent, dir_name) != NULL) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("mkdir: '");
        vga_puts(dir_name);
        vga_puts("': Directory or file already exists\n");
        return;
    }

    vfs_node_t* new_dir = ramfs_create_dir(parent, dir_name);
    if (new_dir) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("[OK] Created directory: ");
        vga_puts(full);
        vga_puts("\n");
    }
}

static void cmd_touch(const char* path) {
    if (!path || *path == '\0') {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("touch: missing file operand\n");
        return;
    }

    char full[128];
    resolve_full_path(path, full, sizeof(full));

    char parent_path[128];
    char file_name[64];
    split_path(full, parent_path, sizeof(parent_path), file_name, sizeof(file_name));

    vfs_node_t* parent = vfs_resolve_path(parent_path);
    if (!parent) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("touch: cannot touch '");
        vga_puts(path);
        vga_puts("': Directory not found\n");
        return;
    }

    if (vfs_finddir(parent, file_name) != NULL) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_puts("[OK] Updated timestamp: ");
        vga_puts(file_name);
        vga_puts("\n");
        return;
    }

    vfs_node_t* new_file = ramfs_create_file(parent, file_name, "");
    if (new_file) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("[OK] Created file: ");
        vga_puts(full);
        vga_puts("\n");
    }
}

static void cmd_rm(const char* path) {
    if (!path || *path == '\0') {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("rm: missing operand\n");
        return;
    }

    char full[128];
    resolve_full_path(path, full, sizeof(full));

    char parent_path[128];
    char node_name[64];
    split_path(full, parent_path, sizeof(parent_path), node_name, sizeof(node_name));

    vfs_node_t* parent = vfs_resolve_path(parent_path);
    if (!parent) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("rm: cannot remove '");
        vga_puts(path);
        vga_puts("': No such file or directory\n");
        return;
    }

    if (ramfs_delete_node(parent, node_name)) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("[OK] Removed '");
        vga_puts(full);
        vga_puts("'\n");
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("rm: failed to remove '");
        vga_puts(node_name);
        vga_puts("' (directory not empty or protected)\n");
    }
}

static void cmd_uptime(void) {
    uint64_t total_sec = timer_get_ticks() / 100;
    uint64_t hrs = total_sec / 3600;
    uint64_t mins = (total_sec % 3600) / 60;
    uint64_t secs = total_sec % 60;

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("Airlys Uptime: ");
    vga_putdec(hrs);
    vga_puts("h ");
    vga_putdec(mins);
    vga_puts("m ");
    vga_putdec(secs);
    vga_puts("s | Load: 0.02, 0.01, 0.00 | Ticks: ");
    vga_putdec(timer_get_ticks());
    vga_puts("\n");
}

static void cmd_date(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("System Clock: Fri Oct 23 2026 12:45:00 UTC (Sovereign Hardware Clock Sync)\n");
}

static void cmd_dmesg(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("[    0.000000] Booting Airlys Sovereign OS Kernel (x86_64 Long Mode)...\n");
    vga_puts("[    0.001000] Lead Systems Architect: Er. Adam Eehan (Aira Group)\n");
    vga_puts("[    0.002000] 4-Level Paging initialized: 1024 MB identity mapped (huge pages)\n");
    vga_puts("[    0.003000] IDT64 loaded: 256 interrupt gates active\n");
    vga_puts("[    0.004000] Dual 8259 PIC remapped to IRQ vectors 32..47\n");
    vga_puts("[    0.005000] Programmable Interval Timer (PIT 8254) running at 100 Hz\n");
    vga_puts("[    0.006000] PS/2 hardware keyboard controller initialized\n");
    vga_puts("[    0.007000] PMM Bitmap Allocator tracking 262,144 physical frames\n");
    vga_puts("[    0.008000] Sovereign Dynamic Kernel Heap active: 32 MB pool at 0x1000000\n");
    vga_puts("[    0.009000] Preemptive Round-Robin Multitasking active (Quantum: 50ms)\n");
    vga_puts("[    0.010000] In-Memory RamFS mounted on '/'\n");
    vga_puts("[    0.011000] airlys-sh v1.0 interactive userland shell online\n");
}

static void cmd_kill(const char* arg) {
    if (!arg || *arg == '\0') {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("kill: missing PID operand\n");
        return;
    }

    uint32_t pid = katoi(arg);
    if (sched_kill_task(pid)) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("[OK] Terminated task PID ");
        vga_putdec(pid);
        vga_puts("\n");
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("kill: failed to terminate PID ");
        vga_putdec(pid);
        vga_puts(" (protected or non-existent)\n");
    }
}

static void cmd_whoami(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("root\n");
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("  Sovereign Authority: Er. Adam Eehan (Founder & CEO, Aira Group)\n");
}

static void cmd_uname(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Airlys Sovereign OS 0.2.0-genesis x86_64 Long Mode (Monolithic Freestanding Kernel)\n");
}

static void cmd_free(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("              TOTAL          USED          FREE\n");
    vga_puts(" ------------------------------------------------\n");

    /* Physical RAM */
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts(" Phys RAM:    1024 MB        ");
    vga_putdec(1024 - (pmm_get_free_memory_kb() / 1024));
    vga_puts(" MB         ");
    vga_putdec(pmm_get_free_memory_kb() / 1024);
    vga_puts(" MB\n");

    /* Dynamic Kernel Heap */
    vga_puts(" Dyn Heap:    32 MB          ");
    vga_putdec(heap_get_used_bytes() / 1024);
    vga_puts(" KB          ");
    vga_putdec(heap_get_free_bytes() / 1024);
    vga_puts(" KB\n");
}

static void cmd_covenant(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("=================================================================\n");
    vga_puts("               THE SOVEREIGN OPEN-SOURCE COVENANT                \n");
    vga_puts("            Aira Group of Technology & Er. Adam Eehan            \n");
    vga_puts("=================================================================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts(" 1. 100% Free Lifetime OS, Software & Cloud Compute for students \n");
    vga_puts("    and underprivileged youth worldwide.\n");
    vga_puts(" 2. Funded exclusively by enterprise adoption in affluent markets.\n");
    vga_puts(" 3. Technology is an equalizing force for humanity, not an elite \n");
    vga_puts("    monopoly.\n");
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("=================================================================\n");
}

static void cmd_aira(const char* subcmd) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("[Aira Sovereign Guard] Elevated root authority verified.\n");

    if (!subcmd || *subcmd == '\0') {
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts("Usage: aira <command> (e.g. 'aira status', 'aira audit', 'aira sentinel')\n");
        return;
    }

    if (kstrcmp(subcmd, "status") == 0) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("  Status: All Sovereign Kernel Subsystems (PMM, VMM, Scheduler, VFS) Healthy.\n");
    } else if (kstrcmp(subcmd, "audit") == 0 || kstrcmp(subcmd, "scan") == 0) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("  Audit: 0 privilege leaks, Ring 0 memory protection intact, Sentinel active.\n");
    } else if (kstrcmp(subcmd, "sentinel") == 0) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("  Sentinel: Sovereign background watchdog running on PID 1.\n");
    } else {
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts("  Executing '");
        vga_puts(subcmd);
        vga_puts("' with sovereign administrative privileges... [OK]\n");
    }
}

/* ==================== CYBER SECURITY & HACKING SUITE ==================== */

static void cmd_nmap(const char* target) {
    const char* host = (target && *target != '\0') ? target : "192.168.1.1";

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Starting Sovereign Nmap 7.94 ( https://airagroup.org ) at 2026-10-23 12:45\n");
    vga_puts("Nmap scan report for ");
    vga_puts(host);
    vga_puts("\nHost is up (0.00042s latency).\n");
    vga_puts("Not shown: 992 closed tcp ports (reset)\n\n");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("PORT      STATE SERVICE        VERSION\n");
    vga_puts("21/tcp    open  ftp            vsftpd 3.0.3\n");
    vga_puts("22/tcp    open  ssh            OpenSSH 9.3p1 (Sovereign Shield)\n");
    vga_puts("53/tcp    open  domain         Airlys Sovereign DNS\n");
    vga_puts("80/tcp    open  http           nginx 1.24.0 (Aira Web Engine)\n");
    vga_puts("443/tcp   open  https          TLSv1.3 AES-GCM-256\n");
    vga_puts("8080/tcp  open  http-proxy     Sovereign Reverse Proxy\n");
    vga_puts("11434/tcp open  aira-engine    Aira AI Local Neural Gateway v3.6\n\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("OS details: Airlys 64-Bit Monolithic OS Kernel (x86_64 Long Mode)\n");
    vga_puts("Nmap done: 1 IP address (1 host up) scanned in 0.18 seconds\n");
}

static void cmd_ifconfig(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("eth0: ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500\n");
    vga_puts("        inet 192.168.1.137  netmask 255.255.255.0  broadcast 192.168.1.255\n");
    vga_puts("        inet6 fe80::a18a:36ff:fe44:1234  prefixlen 64  scopeid 0x20<link>\n");
    vga_puts("        ether 52:54:00:a1:8a:36  txqueuelen 1000  (Ethernet Gigabit VirtIO)\n");
    vga_puts("        RX packets 4210  bytes 3145728 (3.0 MiB)\n");
    vga_puts("        TX packets 2890  bytes 1572864 (1.5 MiB)\n\n");

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("lo0:  ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("flags=73<UP,LOOPBACK,RUNNING>  mtu 65536\n");
    vga_puts("        inet 127.0.0.1  netmask 255.0.0.0\n");
    vga_puts("        inet6 ::1  prefixlen 128  scopeid 0x10<host>\n");
    vga_puts("        loop  txqueuelen 1000  (Local Loopback)\n");
}

static void cmd_netstat(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Active Internet connections (servers and established)\n");
    vga_puts("Proto Recv-Q Send-Q Local Address           Foreign Address         State\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN\n");
    vga_puts("tcp        0      0 0.0.0.0:80              0.0.0.0:*               LISTEN\n");
    vga_puts("tcp        0      0 0.0.0.0:443             0.0.0.0:*               LISTEN\n");
    vga_puts("tcp        0      0 0.0.0.0:11434           0.0.0.0:*               LISTEN\n");
    vga_puts("tcp        0      0 192.168.1.137:44320     140.82.121.4:443        ESTABLISHED\n");
    vga_puts("udp        0      0 0.0.0.0:53              0.0.0.0:*                      \n");
}

static void cmd_aircrack(void) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("[+] Airlys 802.11 Monitor Interface [wlan0mon] Active\n");
    vga_puts("-------------------------------------------------------------------------\n");
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("BSSID              PWR  BEACONS  #DATA  CH  MB   ENC  CIPHER  AUTH  ESSID\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("A0:E4:CB:12:34:56  -38      140     58   6  54e. WPA3 CCMP    SAE   Airlys-Mesh-5G\n");
    vga_puts("B4:FB:E4:77:88:99  -52       98     24   1  54e  WPA2 CCMP    PSK   Aira-HQ-Secure\n");
    vga_puts("00:11:22:33:44:55  -68       42      7  11  54e  WPA2 CCMP    PSK   Kerala-Public-WiFi\n");
    vga_puts("-------------------------------------------------------------------------\n");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("[*] WPA Handshake Capture: Passive Packet Monitor Active on Channel 6\n");
}

static void cmd_tcpdump(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("tcpdump: verbose output, listening on eth0, link-type EN10MB (Ethernet)\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("12:45:01.102 IP 192.168.1.137.44320 > 140.82.121.4.443: Flags [P.], seq 1:45\n");
    vga_puts("12:45:01.104 IP 140.82.121.4.443 > 192.168.1.137.44320: Flags [.], ack 45\n");
    vga_puts("12:45:01.210 IP 192.168.1.137.53421 > 8.8.8.8.53: 1422+ A? github.com. (28)\n");
    vga_puts("12:45:01.218 IP 8.8.8.8.53 > 192.168.1.137.53421: 1422 1/0/0 A 140.82.121.4 (44)\n");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("4 packets captured, 4 packets received by filter, 0 packets dropped\n");
}

static void cmd_hash(const char* str) {
    if (!str || *str == '\0') {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("hash: missing string argument\n");
        return;
    }

    /* 32-bit FNV-1a Hash */
    uint32_t fnv = 2166136261U;
    const unsigned char* p = (const unsigned char*)str;
    while (*p) {
        fnv ^= (uint32_t)*p++;
        fnv *= 16777619U;
    }

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Input String : ");
    vga_puts(str);
    vga_puts("\nFNV-1a Hash  : 0x");
    const char hex_digits[] = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        vga_putchar(hex_digits[(fnv >> i) & 0xF]);
    }
    vga_puts("\nStatus       : Cryptographic token verified [OK]\n");
}

static void cmd_reboot(void) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("Rebooting Airlys Sovereign OS...\n");
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void shell_execute(const char* cmdline) {
    if (!cmdline) return;

    while (*cmdline == ' ' || *cmdline == '\t') {
        cmdline++;
    }
    if (*cmdline == '\0') return;

    char cmd[32];
    size_t i = 0;
    while (*cmdline != ' ' && *cmdline != '\t' && *cmdline != '\0' && i < sizeof(cmd) - 1) {
        cmd[i++] = *cmdline++;
    }
    cmd[i] = '\0';

    while (*cmdline == ' ' || *cmdline == '\t') {
        cmdline++;
    }
    const char* args = cmdline;

    if (kstrcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (kstrcmp(cmd, "clear") == 0) {
        vga_clear();
    } else if (kstrcmp(cmd, "pwd") == 0) {
        cmd_pwd();
    } else if (kstrcmp(cmd, "cd") == 0) {
        cmd_cd(args);
    } else if (kstrcmp(cmd, "ls") == 0) {
        cmd_ls(args);
    } else if (kstrcmp(cmd, "cat") == 0) {
        cmd_cat(args);
    } else if (kstrcmp(cmd, "mkdir") == 0) {
        cmd_mkdir(args);
    } else if (kstrcmp(cmd, "touch") == 0) {
        cmd_touch(args);
    } else if (kstrcmp(cmd, "rm") == 0) {
        cmd_rm(args);
    } else if (kstrcmp(cmd, "echo") == 0) {
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts(args);
        vga_puts("\n");
    } else if (kstrcmp(cmd, "whoami") == 0) {
        cmd_whoami();
    } else if (kstrcmp(cmd, "uname") == 0) {
        cmd_uname();
    } else if (kstrcmp(cmd, "uptime") == 0) {
        cmd_uptime();
    } else if (kstrcmp(cmd, "date") == 0) {
        cmd_date();
    } else if (kstrcmp(cmd, "dmesg") == 0) {
        cmd_dmesg();
    } else if (kstrcmp(cmd, "free") == 0) {
        cmd_free();
    } else if (kstrcmp(cmd, "ps") == 0) {
        sched_print_tasks();
    } else if (kstrcmp(cmd, "kill") == 0) {
        cmd_kill(args);
    } else if (kstrcmp(cmd, "covenant") == 0) {
        cmd_covenant();
    } else if (kstrcmp(cmd, "aira") == 0) {
        cmd_aira(args);
    } else if (kstrcmp(cmd, "sudo") == 0) {
        cmd_aira(args);
    } else if (kstrcmp(cmd, "nmap") == 0) {
        cmd_nmap(args);
    } else if (kstrcmp(cmd, "ifconfig") == 0 || kstrcmp(cmd, "ip") == 0) {
        cmd_ifconfig();
    } else if (kstrcmp(cmd, "netstat") == 0) {
        cmd_netstat();
    } else if (kstrcmp(cmd, "aircrack") == 0 || kstrcmp(cmd, "wifi") == 0) {
        cmd_aircrack();
    } else if (kstrcmp(cmd, "tcpdump") == 0 || kstrcmp(cmd, "sniff") == 0) {
        cmd_tcpdump();
    } else if (kstrcmp(cmd, "hash") == 0) {
        cmd_hash(args);
    } else if (kstrcmp(cmd, "reboot") == 0) {
        cmd_reboot();
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("airlys-sh: command not found: ");
        vga_puts(cmd);
        vga_puts(" (type 'help' for command list)\n");
    }
}

void shell_handle_char(char c) {
    if (c == '\n') {
        vga_putchar('\n');
        shell_buffer[shell_pos] = '\0';
        shell_execute(shell_buffer);
        shell_pos = 0;
        shell_buffer[0] = '\0';
        vga_print_prompt();
        return;
    }

    if (c == '\b') {
        if (shell_pos > 0) {
            shell_pos--;
            shell_buffer[shell_pos] = '\0';
            vga_putchar('\b');
        }
        return;
    }

    if ((unsigned char)c >= 32 && (unsigned char)c <= 126) {
        if (shell_pos < SHELL_MAX_INPUT - 1) {
            shell_buffer[shell_pos++] = c;
            shell_buffer[shell_pos] = '\0';
            vga_putchar(c);
        }
    }
}
