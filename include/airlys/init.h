#ifndef AIRLYS_INIT_H
#define AIRLYS_INIT_H

/*
 * Airlys Operating System Kernel - Kernel Init & System UTS Metadata
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define __init
#define __initdata

#define __stringify_1(x...) #x
#define __stringify(x...)   __stringify_1(x)

#define UTS_SYSNAME    "Airlys"
#define UTS_NODENAME   "airlys-enterprise"
#define UTS_RELEASE    "0.2.0-sovereign"
#define UTS_VERSION    "#1 SMP PREEMPT Fri Oct 23 2026 x86_64"
#define UTS_MACHINE    "x86_64"
#define UTS_DOMAINNAME "airagroup.internal"

struct new_utsname {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char domainname[65];
};

extern const char airlys_banner[];
extern const char linux_banner[];
extern struct new_utsname init_uts_ns;

/* Calibrate BogoMIPS and delay routines */
extern unsigned long loops_per_jiffy;
void calibrate_delay(void);
void udelay(uint64_t usecs);
void mdelay(uint64_t msecs);

/* Kernel Subsystems Coordinator */
void airlys_early_init(void);
void airlys_subsystems_init(void);

#endif /* AIRLYS_INIT_H */
