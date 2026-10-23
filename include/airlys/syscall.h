#ifndef AIRLYS_SYSCALL_H
#define AIRLYS_SYSCALL_H

#include <airlys/types.h>

#define SYS_YIELD       0
#define SYS_EXIT        1
#define SYS_SLEEP       2
#define SYS_PRINT       3
#define SYS_GETPID      4

/* Initialize System Call infrastructure (MSRs / Dispatcher) */
void syscall_init(void);

/* Kernel System Call Dispatcher */
int64_t syscall_dispatch(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3);

#endif /* AIRLYS_SYSCALL_H */
