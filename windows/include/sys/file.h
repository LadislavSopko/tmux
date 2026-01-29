/*
 * sys/file.h - Windows compatibility shim for file locking
 */

#ifndef _SYS_FILE_H_WIN32
#define _SYS_FILE_H_WIN32

#include <io.h>
#include <fcntl.h>

/* flock operations */
#define LOCK_SH 1  /* Shared lock */
#define LOCK_EX 2  /* Exclusive lock */
#define LOCK_NB 4  /* Non-blocking */
#define LOCK_UN 8  /* Unlock */

/* flock stub - actual implementation would use LockFileEx */
static __inline int flock(int fd, int operation)
{
    (void)fd; (void)operation;
    /* TODO: implement with LockFileEx/UnlockFileEx */
    return 0;
}

#endif /* _SYS_FILE_H_WIN32 */
