#ifndef AIRLYS_PRINTK_H
#define AIRLYS_PRINTK_H

#include <airlys/types.h>

/* Linux Kernel Log Levels (include/linux/kern_levels.h) */
#define KERN_SOH        "\001"
#define KERN_EMERG      KERN_SOH "0"    /* system is unusable */
#define KERN_ALERT      KERN_SOH "1"    /* action must be taken immediately */
#define KERN_CRIT       KERN_SOH "2"    /* critical conditions */
#define KERN_ERR        KERN_SOH "3"    /* error conditions */
#define KERN_WARNING    KERN_SOH "4"    /* warning conditions */
#define KERN_NOTICE     KERN_SOH "5"    /* normal but significant condition */
#define KERN_INFO       KERN_SOH "6"    /* informational */
#define KERN_DEBUG      KERN_SOH "7"    /* debug-level messages */

#define PRINTK_BUF_SIZE 16384           /* 16 KB Ring Buffer for dmesg */

/* Standard Linux formatted kernel logger */
int printk(const char* fmt, ...);

/* Access kernel ring-buffer (used by dmesg) */
void printk_dump_dmesg(void);

#endif /* AIRLYS_PRINTK_H */
