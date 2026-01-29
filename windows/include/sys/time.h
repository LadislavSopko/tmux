/*
 * sys/time.h - Windows compatibility shim
 */

#ifndef _SYS_TIME_H_WIN32
#define _SYS_TIME_H_WIN32

#include <winsock2.h>  /* For struct timeval */
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* timezone is defined in compat-win32.h */
#ifndef _TIMEZONE_DEFINED
#define _TIMEZONE_DEFINED
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif

/* gettimeofday implementation */
static inline int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME ft;
    ULARGE_INTEGER uli;
    static const ULONGLONG EPOCH_DIFF = 116444736000000000ULL;

    if (tv) {
        GetSystemTimeAsFileTime(&ft);
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        uli.QuadPart -= EPOCH_DIFF;
        tv->tv_sec = (long)(uli.QuadPart / 10000000ULL);
        tv->tv_usec = (long)((uli.QuadPart % 10000000ULL) / 10);
    }
    if (tz) {
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = 0;
    }
    return 0;
}

/* timeradd, timersub, etc. */
#ifndef timeradd
#define timeradd(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
        if ((result)->tv_usec >= 1000000) { \
            ++(result)->tv_sec; \
            (result)->tv_usec -= 1000000; \
        } \
    } while (0)
#endif

#ifndef timersub
#define timersub(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) { \
            --(result)->tv_sec; \
            (result)->tv_usec += 1000000; \
        } \
    } while (0)
#endif

#ifndef timercmp
#define timercmp(a, b, CMP) \
    (((a)->tv_sec == (b)->tv_sec) ? \
     ((a)->tv_usec CMP (b)->tv_usec) : \
     ((a)->tv_sec CMP (b)->tv_sec))
#endif

#ifndef timerclear
#define timerclear(tvp) ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#endif

#ifndef timerisset
#define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SYS_TIME_H_WIN32 */
