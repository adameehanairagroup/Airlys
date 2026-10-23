#ifndef AIRLYS_KERNEL_H
#define AIRLYS_KERNEL_H

#include <airlys/types.h>
#include <airlys/vga.h>

#define AIRLYS_VERSION_MAJOR 0
#define AIRLYS_VERSION_MINOR 1
#define AIRLYS_VERSION_PATCH 0
#define AIRLYS_CODENAME      "Genesis"

void kernel_main(uint64_t magic, uint64_t addr);
void kernel_panic(const char* message);

#endif /* AIRLYS_KERNEL_H */
