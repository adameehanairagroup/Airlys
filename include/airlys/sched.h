#ifndef AIRLYS_SCHED_H
#define AIRLYS_SCHED_H

#include <airlys/types.h>

#define MAX_TASKS           32
#define TASK_STACK_SIZE     (16 * 1024) /* 16 KB Kernel Stack per task */
#define DEFAULT_TIME_SLICE  5           /* 5 timer ticks (50ms) */

typedef enum {
    TASK_STATE_UNUSED = 0,
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_SLEEPING,
    TASK_STATE_TERMINATED
} task_state_t;

typedef struct task {
    uint32_t pid;
    char name[32];
    task_state_t state;
    uintptr_t rsp;          /* Saved kernel stack pointer */
    uintptr_t stack_base;   /* Base allocated stack pointer */
    uint64_t sleep_until;   /* Target tick count for sleep wakeup */
    uint64_t time_slice;    /* Ticks remaining in current execution burst */
    uint64_t total_ticks;   /* Cumulative execution ticks */
    struct task* next;      /* Next pointer in circular queue */
} task_t;

/* Initialize preemptive scheduler */
void sched_init(void);

/* Create a new kernel task thread */
task_t* task_create(const char* name, void (*entry_point)(void));

/* Trigger context switch to next ready task */
void schedule(void);

/* Voluntarily yield the remaining time slice */
void task_yield(void);

/* Terminate the currently running task */
void task_exit(void);

/* Put the current task to sleep for specified ticks (1 tick = 10ms) */
void task_sleep(uint64_t ticks);

/* Preemptive timer tick handler called from PIT ISR */
void sched_timer_tick(void);

/* Get currently executing task pointer */
task_t* sched_get_current(void);

/* Terminate a specific task by PID (returns true if killed) */
bool sched_kill_task(uint32_t pid);

/* Print all active tasks in system to console */
void sched_print_tasks(void);

/* Low-level assembly context switch */
extern void task_switch(uintptr_t* prev_rsp, uintptr_t* next_rsp);

#endif /* AIRLYS_SCHED_H */
