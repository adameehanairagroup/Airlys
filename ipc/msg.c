#include <airlys/ipc.h>
#include <airlys/string.h>

static msg_queue_t g_msg_queues[MAX_MSG_QUEUES];

int msgget(int32_t key) {
    int empty_slot = -1;
    for (int i = 0; i < MAX_MSG_QUEUES; i++) {
        if (g_msg_queues[i].active && g_msg_queues[i].msqid == key) {
            return i;
        }
        if (!g_msg_queues[i].active && empty_slot == -1) {
            empty_slot = i;
        }
    }

    if (empty_slot != -1) {
        g_msg_queues[empty_slot].active = true;
        g_msg_queues[empty_slot].msqid = key;
        g_msg_queues[empty_slot].msg_count = 0;
        return empty_slot;
    }
    return -1;
}

int msgsnd(int msqid, const msg_buf_t* msg) {
    if (msqid < 0 || msqid >= MAX_MSG_QUEUES || !g_msg_queues[msqid].active || !msg) {
        return -1;
    }

    msg_queue_t* q = &g_msg_queues[msqid];
    if (q->msg_count >= 16) {
        return -1; /* Queue full */
    }

    kmemcpy(&q->messages[q->msg_count], msg, sizeof(msg_buf_t));
    q->msg_count++;
    return 0;
}

int msgrcv(int msqid, msg_buf_t* msg, int64_t msgtyp) {
    if (msqid < 0 || msqid >= MAX_MSG_QUEUES || !g_msg_queues[msqid].active || !msg) {
        return -1;
    }

    msg_queue_t* q = &g_msg_queues[msqid];
    if (q->msg_count == 0) {
        return -1;
    }

    for (uint32_t i = 0; i < q->msg_count; i++) {
        if (msgtyp == 0 || q->messages[i].mtype == msgtyp) {
            kmemcpy(msg, &q->messages[i], sizeof(msg_buf_t));

            /* Shift remaining messages */
            for (uint32_t j = i; j < q->msg_count - 1; j++) {
                q->messages[j] = q->messages[j + 1];
            }
            q->msg_count--;
            return (int)msg->msize;
        }
    }
    return -1;
}
