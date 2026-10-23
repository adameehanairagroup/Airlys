#include <airlys/sched.h>
#include <airlys/heap.h>
#include <airlys/vga.h>
#include <airlys/timer.h>
#include <airlys/kernel.h>

static task_t tasks[MAX_TASKS];
static task_t* current_task = NULL;
static bool sched_active = false;
static uint32_t next_pid = 1;

static void kstrncpy(char* dest, const char* src, size_t n) {
    size_t i = 0;
    while (i < n - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void sched_init(void) {
    for (size_t i = 0; i < MAX_TASKS; i++) {
        tasks[i].pid = 0;
        tasks[i].state = TASK_STATE_UNUSED;
        tasks[i].rsp = 0;
        tasks[i].stack_base = 0;
        tasks[i].sleep_until = 0;
        tasks[i].time_slice = DEFAULT_TIME_SLICE;
        tasks[i].total_ticks = 0;
        tasks[i].next = NULL;
    }

    /* Initialize Task 0 as current running kernel main thread */
    tasks[0].pid = 0;
    kstrncpy(tasks[0].name, "airlys_core", sizeof(tasks[0].name));
    tasks[0].state = TASK_STATE_RUNNING;
    tasks[0].time_slice = DEFAULT_TIME_SLICE;
    tasks[0].stack_base = 0; /* Uses boot stack */
    tasks[0].rsp = 0;

    current_task = &tasks[0];
    sched_active = true;
}

task_t* task_create(const char* name, void (*entry_point)(void)) {
    if (!entry_point) return NULL;

    /* Find free task slot */
    task_t* new_task = NULL;
    for (size_t i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_STATE_UNUSED || tasks[i].state == TASK_STATE_TERMINATED) {
            new_task = &tasks[i];
            break;
        }
    }

    if (!new_task) {
        return NULL; /* Task table full */
    }

    /* Allocate 16 KB Kernel Stack */
    uintptr_t stack = (uintptr_t)kmalloc(TASK_STACK_SIZE);
    if (!stack) {
        return NULL; /* Out of memory */
    }

    new_task->pid = next_pid++;
    kstrncpy(new_task->name, name ? name : "task", sizeof(new_task->name));
    new_task->stack_base = stack;
    new_task->sleep_until = 0;
    new_task->time_slice = DEFAULT_TIME_SLICE;
    new_task->total_ticks = 0;

    /* Setup initial stack frame (descending from stack top) */
    uint64_t* sp = (uint64_t*)(stack + TASK_STACK_SIZE);

    /* 1. Return address if task entry_point returns -> task_exit */
    *(--sp) = (uint64_t)task_exit;

    /* 2. Target RIP popped by ret in task_switch */
    *(--sp) = (uint64_t)entry_point;

    /* 3. RFLAGS: 0x202 = Interrupts Enabled (IF=1) */
    *(--sp) = 0x202ULL;

    /* 4. Callee-saved registers (rbx, rbp, r12..r15) */
    *(--sp) = 0; /* rbx */
    *(--sp) = 0; /* rbp */
    *(--sp) = 0; /* r12 */
    *(--sp) = 0; /* r13 */
    *(--sp) = 0; /* r14 */
    *(--sp) = 0; /* r15 */

    new_task->rsp = (uintptr_t)sp;
    new_task->state = TASK_STATE_READY;

    return new_task;
}

void schedule(void) {
    if (!sched_active || !current_task) return;

    uint64_t current_ticks = timer_get_ticks();

    /* Wake up any sleeping tasks whose timers expired */
    for (size_t i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_STATE_SLEEPING && current_ticks >= tasks[i].sleep_until) {
            tasks[i].state = TASK_STATE_READY;
        }
    }

    /* Find next ready task (Round-Robin) */
    size_t current_idx = (size_t)(current_task - &tasks[0]);
    task_t* next_task = NULL;

    for (size_t i = 1; i <= MAX_TASKS; i++) {
        size_t idx = (current_idx + i) % MAX_TASKS;
        if (tasks[idx].state == TASK_STATE_READY) {
            next_task = &tasks[idx];
            break;
        }
    }

    /* If no other ready task, check if current is still running/ready */
    if (!next_task) {
        if (current_task->state == TASK_STATE_RUNNING || current_task->state == TASK_STATE_READY) {
            next_task = current_task;
        } else {
            /* Fallback to PID 0 (kernel main / idle task) */
            next_task = &tasks[0];
            if (next_task->state != TASK_STATE_RUNNING) {
                next_task->state = TASK_STATE_READY;
            }
        }
    }

    if (next_task == current_task) {
        current_task->state = TASK_STATE_RUNNING;
        return;
    }

    /* Perform Context Switch */
    task_t* prev_task = current_task;
    if (prev_task->state == TASK_STATE_RUNNING) {
        prev_task->state = TASK_STATE_READY;
    }

    next_task->state = TASK_STATE_RUNNING;
    next_task->time_slice = DEFAULT_TIME_SLICE;
    current_task = next_task;

    task_switch(&prev_task->rsp, &next_task->rsp);
}

void task_yield(void) {
    schedule();
}

void task_exit(void) {
    if (current_task) {
        current_task->state = TASK_STATE_TERMINATED;
        if (current_task->stack_base != 0) {
            kfree((void*)current_task->stack_base);
            current_task->stack_base = 0;
        }
    }
    schedule();

    /* Unreachable halt loop */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void task_sleep(uint64_t ticks) {
    if (!current_task) return;
    current_task->sleep_until = timer_get_ticks() + ticks;
    current_task->state = TASK_STATE_SLEEPING;
    schedule();
}

void sched_timer_tick(void) {
    if (!sched_active || !current_task) return;

    current_task->total_ticks++;

    if (current_task->time_slice > 0) {
        current_task->time_slice--;
    }

    if (current_task->time_slice == 0) {
        current_task->time_slice = DEFAULT_TIME_SLICE;
        schedule();
    }
}

task_t* sched_get_current(void) {
    return current_task;
}

void sched_print_tasks(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("\n PID  STATE       NAME            TICKS\n");
    vga_puts(" ----------------------------------------\n");

    for (size_t i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_STATE_UNUSED) continue;

        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts(" ");
        vga_putdec(tasks[i].pid);
        vga_puts("    ");

        switch (tasks[i].state) {
            case TASK_STATE_RUNNING:
                vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
                vga_puts("RUNNING     ");
                break;
            case TASK_STATE_READY:
                vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
                vga_puts("READY       ");
                break;
            case TASK_STATE_SLEEPING:
                vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                vga_puts("SLEEPING    ");
                break;
            case TASK_STATE_TERMINATED:
                vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
                vga_puts("TERMINATED  ");
                break;
            default:
                vga_puts("UNKNOWN     ");
                break;
        }

        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_puts(tasks[i].name);
        for (size_t j = 0; tasks[i].name[j] != '\0' && j < 16; j++) {
            /* padding */
        }
        vga_puts("    ");
        vga_putdec(tasks[i].total_ticks);
        vga_puts("\n");
    }
}
