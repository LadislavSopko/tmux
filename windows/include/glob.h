/*
 * glob.h - Windows compatibility shim for POSIX glob
 *
 * Provides glob() pattern matching for file paths.
 * Windows implementation using FindFirstFile/FindNextFile.
 */

#ifndef COMPAT_GLOB_H
#define COMPAT_GLOB_H

#ifdef _WIN32

#include <stdlib.h>

/* glob flags */
#define GLOB_ERR        0x0001  /* Return on read errors */
#define GLOB_MARK       0x0002  /* Append / to directories */
#define GLOB_NOSORT     0x0004  /* Don't sort names */
#define GLOB_NOCHECK    0x0008  /* Return pattern if no matches */
#define GLOB_NOESCAPE   0x0010  /* Disable backslash escaping */
#define GLOB_APPEND     0x0020  /* Append to existing result */
#define GLOB_BRACE      0x0040  /* Expand braces */
#define GLOB_TILDE      0x0080  /* Expand ~ */

/* glob error returns */
#define GLOB_NOSPACE    1       /* Memory allocation failure */
#define GLOB_ABORTED    2       /* Read error */
#define GLOB_NOMATCH    3       /* No matches found */

typedef struct {
    size_t gl_pathc;    /* Count of paths matched */
    char **gl_pathv;    /* List of matched paths */
    size_t gl_offs;     /* Reserved slots at beginning */
} glob_t;

/*
 * glob - find pathnames matching a pattern
 *
 * Stub implementation - returns GLOB_NOMATCH for now.
 * Full implementation would use FindFirstFile/FindNextFile.
 */
static __inline int glob(const char *pattern, int flags,
                         int (*errfunc)(const char *, int), glob_t *pglob)
{
    (void)pattern; (void)flags; (void)errfunc;
    if (pglob) {
        pglob->gl_pathc = 0;
        pglob->gl_pathv = NULL;
        pglob->gl_offs = 0;
    }
    /* TODO: Implement using FindFirstFile/FindNextFile */
    return GLOB_NOMATCH;
}

/*
 * globfree - free memory from glob
 */
static __inline void globfree(glob_t *pglob)
{
    if (pglob && pglob->gl_pathv) {
        size_t i;
        for (i = 0; i < pglob->gl_pathc; i++) {
            free(pglob->gl_pathv[i]);
        }
        free(pglob->gl_pathv);
        pglob->gl_pathv = NULL;
        pglob->gl_pathc = 0;
    }
}

#endif /* _WIN32 */
#endif /* COMPAT_GLOB_H */
