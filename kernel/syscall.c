#include <airlys/syscall.h>
#include <airlys/sched.h>
#include <airlys/vga.h>

int64_t syscall_dispatch(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    (void)arg2;
    (void)arg3;

    switch (num) {
        case SYS_YIELD:
            task_yield();
            return 0;

        case SYS_EXIT:
            task_exit();
            return 0;

        case SYS_SLEEP:
            task_sleep(arg1);
            return 0;

        case SYS_PRINT:
            if (arg1 != 0) {
                vga_puts((const char*)arg1);
            }
            return 0;

        case SYS_GETPID:
            if (sched_get_current()) {
                return (int64_t)sched_get_current()->pid;
            }
            return -1;

        default:
            return -1; /* Unknown syscall */
    }
}

void syscall_init(void) {
    /* Ready the syscall dispatch table */
}
