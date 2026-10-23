#include <airlys/ipc.h>
#include <airlys/string.h>
#include <airlys/printk.h>

int sample_pipe_demo(void) {
    int fds[2];
    if (pipe_create(fds) != 0) {
        printk(KERN_ERR "sample: Failed to create pipe\n");
        return -1;
    }

    const char* msg = "Hello from sovereign IPC pipe!";
    pipe_write(fds[1], msg, kstrlen(msg));

    char rx_buf[64];
    kmemset(rx_buf, 0, sizeof(rx_buf));
    ssize_t bytes = pipe_read(fds[0], rx_buf, sizeof(rx_buf) - 1);

    if (bytes > 0) {
        printk(KERN_INFO "sample: Pipe test passed! Read '%s' (%d bytes)\n", rx_buf, (int)bytes);
    }

    pipe_close(fds[0]);
    pipe_close(fds[1]);
    return 0;
}
