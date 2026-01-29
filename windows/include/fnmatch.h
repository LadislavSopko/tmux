/*
 * fnmatch.h - Windows compatibility shim
 */

#ifndef _FNMATCH_H_WIN32
#define _FNMATCH_H_WIN32

#define FNM_NOMATCH     1
#define FNM_PATHNAME    (1 << 0)
#define FNM_NOESCAPE    (1 << 1)
#define FNM_PERIOD      (1 << 2)
#define FNM_CASEFOLD    (1 << 4)

/* Simple fnmatch implementation */
static inline int fnmatch(const char *pattern, const char *string, int flags)
{
    (void)flags;
    const char *p = pattern;
    const char *s = string;

    while (*p && *s) {
        if (*p == '*') {
            p++;
            if (!*p) return 0;
            while (*s) {
                if (fnmatch(p, s, flags) == 0) return 0;
                s++;
            }
            return FNM_NOMATCH;
        } else if (*p == '?') {
            p++;
            s++;
        } else if (*p == *s) {
            p++;
            s++;
        } else {
            return FNM_NOMATCH;
        }
    }

    while (*p == '*') p++;
    return (*p == '\0' && *s == '\0') ? 0 : FNM_NOMATCH;
}

#endif /* _FNMATCH_H_WIN32 */
