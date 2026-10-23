#ifndef AIRLYS_SHELL_H
#define AIRLYS_SHELL_H

#include <airlys/types.h>

#define SHELL_MAX_INPUT 256

/* Initialize the interactive userland shell */
void shell_init(void);

/* Handle keyboard character input from hardware keyboard driver */
void shell_handle_char(char c);

/* Parse and execute a shell command line */
void shell_execute(const char* cmdline);

#endif /* AIRLYS_SHELL_H */
