#ifndef AIRLYS_AIRALANG_H
#define AIRLYS_AIRALANG_H

/*
 * Airlys Operating System Kernel - Native Embedded AiraLang Engine (Phase 6)
 * Language Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define AIRALANG_VERSION "v1.4.0 (Kernel Native Edition)"
#define MAX_AIRA_VARS    32
#define MAX_VAR_NAME     32
#define MAX_VAR_STR      128

typedef enum {
    VAL_NULL = 0,
    VAL_NUM,
    VAL_STR,
    VAL_BOOL
} val_type_t;

typedef struct {
    char       name[MAX_VAR_NAME];
    val_type_t type;
    int64_t    num_val;
    char       str_val[MAX_VAR_STR];
    bool       bool_val;
    bool       allocated;
} aira_var_t;

/* AiraLang Engine API */
void airalang_init(void);
int airalang_eval_line(const char* line);
int airalang_run_script(const char* script);
void airalang_print_vars(void);

#endif /* AIRLYS_AIRALANG_H */
