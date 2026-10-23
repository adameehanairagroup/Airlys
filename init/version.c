#include <airlys/init.h>

const char airlys_banner[] =
    "Airlys Operating System Kernel (version 0.2.0-sovereign)\n"
    "Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)\n"
    "Engineering Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)\n";

const char linux_banner[] =
    "Linux version 0.2.0-airlys (adam@aira-group) (clang version 18.0.0) #1 SMP PREEMPT Fri Oct 23 2026\n";

struct new_utsname init_uts_ns = {
    .sysname    = UTS_SYSNAME,
    .nodename   = UTS_NODENAME,
    .release    = UTS_RELEASE,
    .version    = UTS_VERSION,
    .machine    = UTS_MACHINE,
    .domainname = UTS_DOMAINNAME
};
