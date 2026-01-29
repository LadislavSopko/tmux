/*
 * sys/uio.h - Windows compatibility shim
 */

#ifndef _SYS_UIO_H_WIN32
#define _SYS_UIO_H_WIN32

#include <stddef.h>
#include <io.h>

/* ssize_t definition */
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef long ssize_t;
#endif

/* iovec structure for scatter/gather I/O */
struct iovec {
    void  *iov_base;
    size_t iov_len;
};

/* readv/writev stubs */
static __inline ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
    ssize_t total = 0;
    int i;
    for (i = 0; i < iovcnt; i++) {
        ssize_t n = _read(fd, iov[i].iov_base, (unsigned int)iov[i].iov_len);
        if (n < 0) return n;
        if (n == 0) break;
        total += n;
        if ((size_t)n < iov[i].iov_len) break;
    }
    return total;
}

static __inline ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    ssize_t total = 0;
    int i;
    for (i = 0; i < iovcnt; i++) {
        ssize_t n = _write(fd, iov[i].iov_base, (unsigned int)iov[i].iov_len);
        if (n < 0) return n;
        total += n;
        if ((size_t)n < iov[i].iov_len) break;
    }
    return total;
}

#endif /* _SYS_UIO_H_WIN32 */
