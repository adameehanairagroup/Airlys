#ifndef AIRLYS_IPC_H
#define AIRLYS_IPC_H

/*
 * Airlys Operating System Kernel - Inter-Process Communication (Linux ipc/ Parity)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define PIPE_BUF_SIZE 4096
#define MAX_PIPES     16
#define MAX_MSG_QUEUES 8
#define MAX_MSG_SIZE  256

/* Pipe */
typedef struct {
    uint8_t  buffer[PIPE_BUF_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    bool     readers_open;
    bool     writers_open;
    bool     allocated;
} pipe_t;

/* Semaphore */
typedef struct {
    int32_t count;
    uint32_t waiters;
} semaphore_t;

/* Message Queue */
typedef struct {
    int64_t mtype;
    uint32_t msize;
    uint8_t  mtext[MAX_MSG_SIZE];
} msg_buf_t;

typedef struct {
    int32_t   msqid;
    msg_buf_t messages[16];
    uint32_t  msg_count;
    bool      active;
} msg_queue_t;

/* IPC Subsystem API */
void ipc_init(void);

/* Pipe API */
int pipe_create(int pipefd[2]);
ssize_t pipe_read(int fd, void* buf, size_t count);
ssize_t pipe_write(int fd, const void* buf, size_t count);
int pipe_close(int fd);

/* Semaphore API */
void sema_init(semaphore_t* sem, int32_t val);
void sema_down(semaphore_t* sem);
int sema_down_trylock(semaphore_t* sem);
void sema_up(semaphore_t* sem);

/* Message Queue API */
int msgget(int32_t key);
int msgsnd(int msqid, const msg_buf_t* msg);
int msgrcv(int msqid, msg_buf_t* msg, int64_t msgtyp);

#endif /* AIRLYS_IPC_H */
