/*
 * sys/wait.h - Windows compatibility shim
 */

#ifndef _SYS_WAIT_H_WIN32
#define _SYS_WAIT_H_WIN32

#include "compat-win32.h"

/* Wait macros are defined in compat-win32.h */

/* waitpid stub - actual implementation in proc-win32.c */
static inline pid_t waitpid(pid_t pid, int *status, int options)
{
    (void)pid; (void)status; (void)options;
    /* Real implementation would use WaitForSingleObject */
    return -1;
}

/* wait stub */
static inline pid_t wait(int *status)
{
    return waitpid(-1, status, 0);
}

#endif /* _SYS_WAIT_H_WIN32 */
