#include <airlys/ipc.h>

void sema_init(semaphore_t* sem, int32_t val) {
    if (!sem) return;
    sem->count = val;
    sem->waiters = 0;
}

void sema_down(semaphore_t* sem) {
    if (!sem) return;
    while (sem->count <= 0) {
        sem->waiters++;
        __asm__ volatile ("pause");
    }
    sem->count--;
    if (sem->waiters > 0) sem->waiters--;
}

int sema_down_trylock(semaphore_t* sem) {
    if (!sem) return -1;
    if (sem->count > 0) {
        sem->count--;
        return 0;
    }
    return -1;
}

void sema_up(semaphore_t* sem) {
    if (!sem) return;
    sem->count++;
}
