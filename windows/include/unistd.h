/*
 * unistd.h - Windows compatibility shim
 */

#ifndef _UNISTD_H_WIN32
#define _UNISTD_H_WIN32

#include <io.h>
#include <direct.h>
#include <process.h>
#include <stdlib.h>

/* Already defined in compat-win32.h, but ensure available */
#include "compat-win32.h"

/* Additional constants */
#ifndef _SC_OPEN_MAX
#define _SC_OPEN_MAX 4
#endif

static inline long sysconf(int name)
{
    switch (name) {
    case _SC_OPEN_MAX:
        return 256;
    default:
        return -1;
    }
}

/* sleep in seconds */
static inline unsigned int sleep(unsigned int seconds)
{
    Sleep(seconds * 1000);
    return 0;
}

/* usleep in microseconds */
static inline int usleep(unsigned int usec)
{
    Sleep(usec / 1000);
    return 0;
}

/* Stub for pipe - would need proper implementation */
static inline int pipe(int pipefd[2])
{
    return _pipe(pipefd, 4096, _O_BINARY);
}

/* fork is not available - use CreateProcess via proc-win32 */
#define fork() (-1)

/* getppid */
static inline pid_t getppid(void)
{
    return 0;
}

/* setpgid/getpgid stubs */
static inline int setpgid(pid_t pid, pid_t pgid)
{
    (void)pid; (void)pgid;
    return 0;
}

static inline pid_t getpgid(pid_t pid)
{
    (void)pid;
    return 0;
}

static inline pid_t setsid(void)
{
    return 0;
}

/* fcntl stub */
static inline int fcntl(int fd, int cmd, ...)
{
    (void)fd; (void)cmd;
    return 0;
}

#endif /* _UNISTD_H_WIN32 */
