/*
 * libgen.h - Windows compatibility shim for POSIX libgen
 *
 * Provides basename() and dirname() functions.
 */

#ifndef COMPAT_LIBGEN_H
#define COMPAT_LIBGEN_H

#ifdef _WIN32

#include <string.h>
#include <stdlib.h>

/*
 * basename - return the last component of a pathname
 *
 * Note: This modifies the input string and returns a pointer into it.
 */
static __inline char *basename(char *path)
{
    char *p;

    if (path == NULL || *path == '\0')
        return ".";

    /* Remove trailing slashes */
    p = path + strlen(path) - 1;
    while (p > path && (*p == '/' || *p == '\\'))
        *p-- = '\0';

    /* Find last slash */
    p = strrchr(path, '/');
    if (p == NULL)
        p = strrchr(path, '\\');

    if (p == NULL)
        return path;

    return p + 1;
}

/*
 * dirname - return the directory portion of a pathname
 *
 * Note: This modifies the input string.
 */
static __inline char *dirname(char *path)
{
    char *p;

    if (path == NULL || *path == '\0')
        return ".";

    /* Remove trailing slashes */
    p = path + strlen(path) - 1;
    while (p > path && (*p == '/' || *p == '\\'))
        *p-- = '\0';

    /* Find last slash */
    p = strrchr(path, '/');
    if (p == NULL)
        p = strrchr(path, '\\');

    if (p == NULL)
        return ".";

    /* Handle root */
    if (p == path)
        return p[1] = '\0', path;

    *p = '\0';
    return path;
}

#endif /* _WIN32 */
#endif /* COMPAT_LIBGEN_H */
