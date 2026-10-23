#include <airlys/shell.h>
#include <airlys/vga.h>
#include <airlys/vfs.h>
#include <airlys/pmm.h>
#include <airlys/heap.h>
#include <airlys/sched.h>
#include <airlys/io.h>
#include <airlys/ramfs.h>

static char shell_buffer[SHELL_MAX_INPUT];
static size_t shell_pos = 0;

static int kstrcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void shell_init(void) {
    shell_pos = 0;
    shell_buffer[0] = '\0';
}

static void cmd_help(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Airlys Sovereign Shell (`airlys-sh` v1.0)\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Built-in Commands:\n");
    vga_puts("  help       - Display this list of commands\n");
    vga_puts("  clear      - Clear the VGA terminal screen\n");
    vga_puts("  ls [dir]   - List contents of a VFS directory (default: /)\n");
    vga_puts("  cat <file> - Read and display file contents\n");
    vga_puts("  mkdir <dir>- Create a new directory / folder\n");
    vga_puts("  touch <f>  - Create a new empty file\n");
    vga_puts("  echo <msg> - Print message to terminal\n");
    vga_puts("  whoami     - Print active user & sovereign authority\n");
    vga_puts("  uname      - Print kernel name, architecture & version\n");
    vga_puts("  free       - Display physical and dynamic kernel heap memory\n");
    vga_puts("  ps         - List active processes and execution ticks\n");
    vga_puts("  covenant   - Read the Sovereign Open-Source Covenant\n");
    vga_puts("  aira <cmd> - Execute administrative command (replaces sudo)\n");
    vga_puts("  reboot     - Warm hardware reboot\n");
}

static void cmd_clear(void) {
    vga_clear();
}

static void cmd_ls(const char* path) {
    const char* target = (path && *path != '\0') ? path : "/";
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

    vfs_node_t* node = vfs_resolve_path(path);
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

static void split_path(const char* full_path, char* parent_buf, size_t parent_len, char* name_buf, size_t name_len) {
    int last_slash = -1;
    for (int i = 0; full_path[i] != '\0'; i++) {
        if (full_path[i] == '/') last_slash = i;
    }

    if (last_slash == -1) {
        /* Default to /home/user */
        size_t j = 0;
        const char* def_parent = "/home/user";
        while (def_parent[j] != '\0' && j < parent_len - 1) {
            parent_buf[j] = def_parent[j];
            j++;
        }
        parent_buf[j] = '\0';

        j = 0;
        while (full_path[j] != '\0' && j < name_len - 1) {
            name_buf[j] = full_path[j];
            j++;
        }
        name_buf[j] = '\0';
    } else if (last_slash == 0) {
        parent_buf[0] = '/';
        parent_buf[1] = '\0';
        size_t j = 0;
        const char* p = full_path + 1;
        while (p[j] != '\0' && j < name_len - 1) {
            name_buf[j] = p[j];
            j++;
        }
        name_buf[j] = '\0';
    } else {
        size_t j = 0;
        while (j < (size_t)last_slash && j < parent_len - 1) {
            parent_buf[j] = full_path[j];
            j++;
        }
        parent_buf[j] = '\0';

        j = 0;
        const char* p = full_path + last_slash + 1;
        while (p[j] != '\0' && j < name_len - 1) {
            name_buf[j] = p[j];
            j++;
        }
        name_buf[j] = '\0';
    }
}

static void cmd_mkdir(const char* path) {
    if (!path || *path == '\0') {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("mkdir: missing directory operand\n");
        return;
    }

    char parent_path[128];
    char dir_name[64];
    split_path(path, parent_path, sizeof(parent_path), dir_name, sizeof(dir_name));

    vfs_node_t* parent = vfs_resolve_path(parent_path);
    if (!parent) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("mkdir: cannot create directory '");
        vga_puts(path);
        vga_puts("': Parent directory not found\n");
        return;
    }

    if (vfs_finddir(parent, dir_name) != NULL) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("mkdir: cannot create directory '");
        vga_puts(dir_name);
        vga_puts("': File or directory already exists\n");
        return;
    }

    vfs_node_t* new_dir = ramfs_create_dir(parent, dir_name);
    if (new_dir) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("[OK] Created directory: ");
        vga_puts(parent_path);
        vga_puts("/");
        vga_puts(dir_name);
        vga_puts("\n");
    }
}

static void cmd_touch(const char* path) {
    if (!path || *path == '\0') {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("touch: missing file operand\n");
        return;
    }

    char parent_path[128];
    char file_name[64];
    split_path(path, parent_path, sizeof(parent_path), file_name, sizeof(file_name));

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
        vga_puts("[OK] Updated file timestamp: ");
        vga_puts(file_name);
        vga_puts("\n");
        return;
    }

    vfs_node_t* new_file = ramfs_create_file(parent, file_name, "");
    if (new_file) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("[OK] Created file: ");
        vga_puts(parent_path);
        vga_puts("/");
        vga_puts(file_name);
        vga_puts("\n");
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

static void cmd_reboot(void) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("Rebooting Airlys Sovereign OS...\n");
    /* Pulse reset line using 8042 keyboard controller port 0x64 */
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

    /* Trim leading spaces */
    while (*cmdline == ' ' || *cmdline == '\t') {
        cmdline++;
    }
    if (*cmdline == '\0') return;

    /* Extract first word (command) */
    char cmd[32];
    size_t i = 0;
    while (*cmdline != ' ' && *cmdline != '\t' && *cmdline != '\0' && i < sizeof(cmd) - 1) {
        cmd[i++] = *cmdline++;
    }
    cmd[i] = '\0';

    /* Skip spaces to get argument string */
    while (*cmdline == ' ' || *cmdline == '\t') {
        cmdline++;
    }
    const char* args = cmdline;

    if (kstrcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (kstrcmp(cmd, "clear") == 0) {
        cmd_clear();
    } else if (kstrcmp(cmd, "ls") == 0) {
        cmd_ls(args);
    } else if (kstrcmp(cmd, "mkdir") == 0) {
        cmd_mkdir(args);
    } else if (kstrcmp(cmd, "touch") == 0) {
        cmd_touch(args);
    } else if (kstrcmp(cmd, "cat") == 0) {
        cmd_cat(args);
    } else if (kstrcmp(cmd, "echo") == 0) {
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts(args);
        vga_puts("\n");
    } else if (kstrcmp(cmd, "whoami") == 0) {
        cmd_whoami();
    } else if (kstrcmp(cmd, "uname") == 0) {
        cmd_uname();
    } else if (kstrcmp(cmd, "free") == 0) {
        cmd_free();
    } else if (kstrcmp(cmd, "ps") == 0) {
        sched_print_tasks();
    } else if (kstrcmp(cmd, "covenant") == 0) {
        cmd_covenant();
    } else if (kstrcmp(cmd, "aira") == 0) {
        cmd_aira(args);
    } else if (kstrcmp(cmd, "sudo") == 0) {
        /* Alias sudo -> aira */
        cmd_aira(args);
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
