/*
 * regex.h - Windows compatibility shim for POSIX regex
 *
 * Provides minimal POSIX regex interface.
 * TODO: Full implementation using Windows regex or PCRE.
 */

#ifndef COMPAT_REGEX_H
#define COMPAT_REGEX_H

#ifdef _WIN32

#include <stdlib.h>

/* Regex compile flags */
#define REG_EXTENDED    1
#define REG_ICASE       2
#define REG_NOSUB       4
#define REG_NEWLINE     8

/* Regex exec flags */
#define REG_NOTBOL      1
#define REG_NOTEOL      2

/* Regex error codes */
#define REG_NOMATCH     1
#define REG_BADPAT      2
#define REG_ECOLLATE    3
#define REG_ECTYPE      4
#define REG_EESCAPE     5
#define REG_ESUBREG     6
#define REG_EBRACK      7
#define REG_EPAREN      8
#define REG_EBRACE      9
#define REG_BADBR       10
#define REG_ERANGE      11
#define REG_ESPACE      12
#define REG_BADRPT      13

typedef struct {
    size_t re_nsub;     /* Number of parenthesized subexpressions */
    void *re_pcre;      /* Internal: compiled pattern (placeholder) */
} regex_t;

typedef int regoff_t;

typedef struct {
    regoff_t rm_so;     /* Byte offset from string start to substring start */
    regoff_t rm_eo;     /* Byte offset from string start to substring end */
} regmatch_t;

/*
 * regcomp - compile a regular expression
 *
 * Stub implementation - always fails for now.
 */
static __inline int regcomp(regex_t *preg, const char *regex, int cflags)
{
    (void)regex; (void)cflags;
    if (preg) {
        preg->re_nsub = 0;
        preg->re_pcre = NULL;
    }
    /* TODO: Implement using Windows regex or link PCRE */
    return REG_ESPACE;
}

/*
 * regexec - execute a compiled regular expression
 */
static __inline int regexec(const regex_t *preg, const char *string,
                            size_t nmatch, regmatch_t pmatch[], int eflags)
{
    (void)preg; (void)string; (void)nmatch; (void)pmatch; (void)eflags;
    return REG_NOMATCH;
}

/*
 * regerror - return error string
 */
static __inline size_t regerror(int errcode, const regex_t *preg,
                                char *errbuf, size_t errbuf_size)
{
    const char *msg = "regex not implemented";
    (void)preg; (void)errcode;
    if (errbuf && errbuf_size > 0) {
        size_t len = strlen(msg);
        if (len >= errbuf_size)
            len = errbuf_size - 1;
        memcpy(errbuf, msg, len);
        errbuf[len] = '\0';
        return len + 1;
    }
    return strlen(msg) + 1;
}

/*
 * regfree - free compiled pattern
 */
static __inline void regfree(regex_t *preg)
{
    if (preg) {
        preg->re_nsub = 0;
        preg->re_pcre = NULL;
    }
}

#endif /* _WIN32 */
#endif /* COMPAT_REGEX_H */
