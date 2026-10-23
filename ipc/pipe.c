#include <airlys/ipc.h>
#include <airlys/string.h>
#include <airlys/printk.h>

static pipe_t g_pipes[MAX_PIPES];

void ipc_init(void) {
    kmemset(g_pipes, 0, sizeof(g_pipes));
    printk(KERN_INFO "ipc: Unix IPC subsystem online (pipes, semaphores, message queues)\n");
}

int pipe_create(int pipefd[2]) {
    if (!pipefd) return -1;

    for (int i = 0; i < MAX_PIPES; i++) {
        if (!g_pipes[i].allocated) {
            g_pipes[i].allocated = true;
            g_pipes[i].head = 0;
            g_pipes[i].tail = 0;
            g_pipes[i].count = 0;
            g_pipes[i].readers_open = true;
            g_pipes[i].writers_open = true;

            pipefd[0] = i * 2;       /* Read end (even) */
            pipefd[1] = i * 2 + 1;   /* Write end (odd) */
            return 0;
        }
    }
    return -1; /* Out of pipes */
}

ssize_t pipe_read(int fd, void* buf, size_t count) {
    if (fd < 0 || (fd % 2) != 0 || !buf) return -1;
    int index = fd / 2;
    if (index >= MAX_PIPES || !g_pipes[index].allocated) return -1;

    pipe_t* p = &g_pipes[index];
    if (p->count == 0) {
        if (!p->writers_open) return 0; /* EOF */
        return 0; /* Non-blocking or empty */
    }

    size_t to_read = count < p->count ? count : p->count;
    uint8_t* out = (uint8_t*)buf;

    for (size_t i = 0; i < to_read; i++) {
        out[i] = p->buffer[p->head];
        p->head = (p->head + 1) % PIPE_BUF_SIZE;
        p->count--;
    }
    return (ssize_t)to_read;
}

ssize_t pipe_write(int fd, const void* buf, size_t count) {
    if (fd < 0 || (fd % 2) != 1 || !buf) return -1;
    int index = fd / 2;
    if (index >= MAX_PIPES || !g_pipes[index].allocated) return -1;

    pipe_t* p = &g_pipes[index];
    if (!p->readers_open) {
        return -1; /* Broken pipe (SIGPIPE) */
    }

    size_t space = PIPE_BUF_SIZE - p->count;
    size_t to_write = count < space ? count : space;
    const uint8_t* in = (const uint8_t*)buf;

    for (size_t i = 0; i < to_write; i++) {
        p->buffer[p->tail] = in[i];
        p->tail = (p->tail + 1) % PIPE_BUF_SIZE;
        p->count++;
    }
    return (ssize_t)to_write;
}

int pipe_close(int fd) {
    if (fd < 0) return -1;
    int index = fd / 2;
    if (index >= MAX_PIPES || !g_pipes[index].allocated) return -1;

    pipe_t* p = &g_pipes[index];
    if ((fd % 2) == 0) {
        p->readers_open = false;
    } else {
        p->writers_open = false;
    }

    if (!p->readers_open && !p->writers_open) {
        p->allocated = false;
        p->count = 0;
        p->head = p->tail = 0;
    }
    return 0;
}
