/*
 * langinfo.h - Windows compatibility shim
 *
 * Provides nl_langinfo() and related definitions.
 */

#ifndef COMPAT_LANGINFO_H
#define COMPAT_LANGINFO_H

#ifdef _WIN32

typedef int nl_item;

/* nl_langinfo items */
#define CODESET     0
#define D_T_FMT     1
#define D_FMT       2
#define T_FMT       3
#define T_FMT_AMPM  4
#define AM_STR      5
#define PM_STR      6
#define DAY_1       7
#define DAY_2       8
#define DAY_3       9
#define DAY_4       10
#define DAY_5       11
#define DAY_6       12
#define DAY_7       13
#define ABDAY_1     14
#define ABDAY_2     15
#define ABDAY_3     16
#define ABDAY_4     17
#define ABDAY_5     18
#define ABDAY_6     19
#define ABDAY_7     20
#define MON_1       21
#define MON_2       22
#define MON_3       23
#define MON_4       24
#define MON_5       25
#define MON_6       26
#define MON_7       27
#define MON_8       28
#define MON_9       29
#define MON_10      30
#define MON_11      31
#define MON_12      32
#define ABMON_1     33
#define ABMON_2     34
#define ABMON_3     35
#define ABMON_4     36
#define ABMON_5     37
#define ABMON_6     38
#define ABMON_7     39
#define ABMON_8     40
#define ABMON_9     41
#define ABMON_10    42
#define ABMON_11    43
#define ABMON_12    44

static __inline char *nl_langinfo(nl_item item)
{
    switch (item) {
    case CODESET:
        return "UTF-8";  /* Assume UTF-8 */
    case D_T_FMT:
        return "%a %b %e %H:%M:%S %Y";
    case D_FMT:
        return "%m/%d/%y";
    case T_FMT:
        return "%H:%M:%S";
    case T_FMT_AMPM:
        return "%I:%M:%S %p";
    case AM_STR:
        return "AM";
    case PM_STR:
        return "PM";
    default:
        return "";
    }
}

#endif /* _WIN32 */
#endif /* COMPAT_LANGINFO_H */
