#ifndef AIRLYS_AIRASTORE_H
#define AIRLYS_AIRASTORE_H

/*
 * Airlys Operating System Kernel - AiraStore & 4-Tier Sandbox Package Subsystem
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define MAX_STORE_APPS 16

typedef enum {
    SANDBOX_TIER_1_BYTECODE = 1,   /* Static binary scan & SHA-256 verification */
    SANDBOX_TIER_2_CAPABILITIES = 2,/* Strict syscall permissions whitelist */
    SANDBOX_TIER_3_USERLAND = 3,    /* Ring 3 memory space isolation */
    SANDBOX_TIER_4_SENTINEL = 4     /* Live Ring 0 Sentinel hardware watchdog */
} sandbox_tier_t;

typedef struct {
    char     name[32];
    char     version[16];
    char     category[24];
    uint32_t size_kb;
    bool     installed;
    bool     verified;
    sandbox_tier_t sandbox_level;
    char     sha256_hash[65];
} store_package_t;

void airastore_init(void);
void airastore_list(void);
bool airastore_install(const char* name);
bool airastore_remove(const char* name);
bool airastore_verify(const char* name);

#endif /* AIRLYS_AIRASTORE_H */
