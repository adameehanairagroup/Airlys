#include <airlys/sched.h>
#include <airlys/printk.h>

void sample_worker_thread(void) {
    printk(KERN_INFO "sample: Sample kernel thread running in background!\n");
    for (int i = 0; i < 3; i++) {
        task_sleep(100); /* 100 ticks = 1 second */
        printk(KERN_INFO "sample: Worker heartbeat tick %d\n", i + 1);
    }
    printk(KERN_INFO "sample: Sample worker thread finished gracefully.\n");
}
