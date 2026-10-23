#include <airlys/airalang.h>
#include <airlys/string.h>
#include <airlys/vga.h>
#include <airlys/sound.h>
#include <airlys/timer.h>
#include <airlys/crypto.h>
#include <airlys/printk.h>

static aira_var_t g_vars[MAX_AIRA_VARS];

static int64_t parse_int(const char* str) {
    int64_t res = 0;
    int sign = 1;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') { str++; }
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res * sign;
}

static aira_var_t* aira_find_var(const char* name) {
    if (!name) return NULL;
    for (int i = 0; i < MAX_AIRA_VARS; i++) {
        if (g_vars[i].allocated && kstrcmp(g_vars[i].name, name) == 0) {
            return &g_vars[i];
        }
    }
    return NULL;
}

static aira_var_t* aira_alloc_var(const char* name) {
    aira_var_t* existing = aira_find_var(name);
    if (existing) return existing;

    for (int i = 0; i < MAX_AIRA_VARS; i++) {
        if (!g_vars[i].allocated) {
            g_vars[i].allocated = true;
            kstrncpy(g_vars[i].name, name, MAX_VAR_NAME);
            return &g_vars[i];
        }
    }
    return NULL;
}

void airalang_init(void) {
    kmemset(g_vars, 0, sizeof(g_vars));

    /* Pre-seed core sovereign environment variables */
    aira_var_t* v_author = aira_alloc_var("FOUNDER");
    if (v_author) {
        v_author->type = VAL_STR;
        kstrcpy(v_author->str_val, "Er. Adam Eehan");
    }

    aira_var_t* v_ai = aira_alloc_var("CO_PILOT");
    if (v_ai) {
        v_ai->type = VAL_STR;
        kstrcpy(v_ai->str_val, "Sovereign AI Aira 3 Ultra (Engine v3.6)");
    }

    aira_var_t* v_ver = aira_alloc_var("VERSION");
    if (v_ver) {
        v_ver->type = VAL_STR;
        kstrcpy(v_ver->str_val, AIRALANG_VERSION);
    }

    printk(KERN_INFO "airalang: Native AiraLang runtime engine %s initialized\n", AIRALANG_VERSION);
}

void airalang_print_vars(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Active AiraLang Variables in Kernel Memory:\n");
    for (int i = 0; i < MAX_AIRA_VARS; i++) {
        if (g_vars[i].allocated) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_puts("  let ");
            vga_puts(g_vars[i].name);
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            vga_puts(" = ");
            if (g_vars[i].type == VAL_STR) {
                vga_putchar('"');
                vga_puts(g_vars[i].str_val);
                vga_puts("\"\n");
            } else if (g_vars[i].type == VAL_NUM) {
                if (g_vars[i].num_val < 0) {
                    vga_putchar('-');
                    vga_putdec((uint64_t)(-g_vars[i].num_val));
                } else {
                    vga_putdec((uint64_t)g_vars[i].num_val);
                }
                vga_puts("\n");
            } else if (g_vars[i].type == VAL_BOOL) {
                vga_puts(g_vars[i].bool_val ? "true\n" : "false\n");
            }
        }
    }
}

static void eval_print_expr(const char* expr) {
    while (*expr == ' ' || *expr == '\t' || *expr == '(') expr++;

    /* String literal in quotes */
    if (*expr == '"') {
        expr++;
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        while (*expr && *expr != '"') {
            vga_putchar(*expr++);
        }
        vga_puts("\n");
        return;
    }

    /* Check variable */
    char varname[32];
    size_t vi = 0;
    while ((*expr >= 'a' && *expr <= 'z') || (*expr >= 'A' && *expr <= 'Z') || *expr == '_') {
        if (vi < sizeof(varname) - 1) varname[vi++] = *expr;
        expr++;
    }
    varname[vi] = '\0';

    aira_var_t* var = aira_find_var(varname);
    if (var) {
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        if (var->type == VAL_STR) {
            vga_puts(var->str_val);
        } else if (var->type == VAL_NUM) {
            if (var->num_val < 0) {
                vga_putchar('-');
                vga_putdec((uint64_t)(-var->num_val));
            } else {
                vga_putdec((uint64_t)var->num_val);
            }
        } else if (var->type == VAL_BOOL) {
            vga_puts(var->bool_val ? "true" : "false");
        }
        vga_puts("\n");
        return;
    }

    /* Check numeric / arithmetic */
    int64_t val = parse_int(varname[0] ? varname : expr);
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    if (val < 0) {
        vga_putchar('-');
        vga_putdec((uint64_t)(-val));
    } else {
        vga_putdec((uint64_t)val);
    }
    vga_puts("\n");
}

int airalang_eval_line(const char* line) {
    if (!line) return 0;
    while (*line == ' ' || *line == '\t') line++;

    /* Skip blank lines or comments (# or //) */
    if (*line == '\0' || *line == '#' || (line[0] == '/' && line[1] == '/')) {
        return 0;
    }

    /* 1. say <expr> or print(...) */
    if (kstrncmp(line, "say ", 4) == 0) {
        eval_print_expr(line + 4);
        return 0;
    }
    if (kstrncmp(line, "print(", 6) == 0) {
        eval_print_expr(line + 6);
        return 0;
    }

    /* 2. let <var> = <value> */
    if (kstrncmp(line, "let ", 4) == 0) {
        line += 4;
        while (*line == ' ' || *line == '\t') line++;

        char varname[MAX_VAR_NAME];
        size_t vi = 0;
        while (*line && *line != ' ' && *line != '=' && *line != '\t' && vi < sizeof(varname) - 1) {
            varname[vi++] = *line++;
        }
        varname[vi] = '\0';

        while (*line == ' ' || *line == '\t') line++;
        if (*line == '=') line++;
        while (*line == ' ' || *line == '\t') line++;

        aira_var_t* var = aira_alloc_var(varname);
        if (!var) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_puts("AiraLang Error: Variable memory full!\n");
            return -1;
        }

        if (*line == '"') {
            line++;
            size_t si = 0;
            while (*line && *line != '"' && si < sizeof(var->str_val) - 1) {
                var->str_val[si++] = *line++;
            }
            var->str_val[si] = '\0';
            var->type = VAL_STR;
        } else if (kstrncmp(line, "true", 4) == 0) {
            var->type = VAL_BOOL;
            var->bool_val = true;
        } else if (kstrncmp(line, "false", 5) == 0) {
            var->type = VAL_BOOL;
            var->bool_val = false;
        } else {
            var->type = VAL_NUM;
            var->num_val = parse_int(line);
        }

        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts(" [AiraLang] Assigned let ");
        vga_puts(varname);
        vga_puts("\n");
        return 0;
    }

    /* 3. beep(freq, ms) */
    if (kstrncmp(line, "beep(", 5) == 0) {
        line += 5;
        uint32_t freq = (uint32_t)parse_int(line);
        while (*line && *line != ',' && *line != ')') line++;
        uint32_t ms = 150;
        if (*line == ',') {
            line++;
            ms = (uint32_t)parse_int(line);
        }
        if (freq == 0) freq = 440;
        pcspkr_beep(freq, ms);
        return 0;
    }

    /* 4. sleep(ms) */
    if (kstrncmp(line, "sleep(", 6) == 0) {
        line += 6;
        uint32_t ms = (uint32_t)parse_int(line);
        timer_sleep(ms / 10);
        return 0;
    }

    /* 5. sha256("text") */
    if (kstrncmp(line, "sha256(", 7) == 0) {
        line += 7;
        while (*line == ' ' || *line == '"') line++;
        char txt[64];
        size_t ti = 0;
        while (*line && *line != '"' && *line != ')' && ti < sizeof(txt) - 1) {
            txt[ti++] = *line++;
        }
        txt[ti] = '\0';
        char hex[65];
        sha256_string(txt, hex);
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts(hex);
        vga_puts("\n");
        return 0;
    }

    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("AiraLang Syntax Error: Unknown statement '");
    vga_puts(line);
    vga_puts("'\n");
    return -1;
}

int airalang_run_script(const char* script) {
    if (!script) return -1;

    char line_buf[128];
    size_t li = 0;

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("================ [ AiraLang Native Interpreter ] ================\n");

    while (*script) {
        if (*script == '\n' || *script == ';') {
            line_buf[li] = '\0';
            if (li > 0) {
                airalang_eval_line(line_buf);
                li = 0;
            }
            script++;
            continue;
        }

        if (li < sizeof(line_buf) - 1) {
            line_buf[li++] = *script;
        }
        script++;
    }

    if (li > 0) {
        line_buf[li] = '\0';
        airalang_eval_line(line_buf);
    }

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("================ [ Execution Completed Cleanly ] ================\n");
    return 0;
}
