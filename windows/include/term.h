/*
 * term.h - Windows compatibility shim for curses terminal definitions
 *
 * Provides terminal capability definitions.
 * Note: PDCurses doesn't use terminfo/termcap, so many of these are stubs.
 */

#ifndef COMPAT_TERM_H
#define COMPAT_TERM_H

#ifdef _WIN32

#include <curses.h>

/* Terminal capability strings - PDCurses doesn't use these */
#define enter_ca_mode       NULL
#define exit_ca_mode        NULL
#define enter_am_mode       NULL
#define exit_am_mode        NULL
#define cursor_normal       NULL
#define cursor_invisible    NULL
#define cursor_visible      NULL
#define keypad_xmit         NULL
#define keypad_local        NULL
#define clr_eol             NULL
#define clr_eos             NULL
#define clear_screen        NULL
#define cursor_address      NULL
#define cursor_up           NULL
#define cursor_down         NULL
#define cursor_left         NULL
#define cursor_right        NULL
#define cursor_home         NULL
#define enter_bold_mode     NULL
#define exit_attribute_mode NULL
#define enter_underline_mode NULL
#define exit_underline_mode NULL
#define enter_reverse_mode  NULL
#define enter_blink_mode    NULL
#define enter_dim_mode      NULL
#define orig_pair           NULL
#define set_a_foreground    NULL
#define set_a_background    NULL
#define set_foreground      NULL
#define set_background      NULL
#define key_up              NULL
#define key_down            NULL
#define key_left            NULL
#define key_right           NULL
#define key_home            NULL
#define key_end             NULL
#define key_ic              NULL
#define key_dc              NULL
#define key_backspace       NULL
#define key_f1              NULL
#define key_f2              NULL
#define key_f3              NULL
#define key_f4              NULL
#define key_f5              NULL
#define key_f6              NULL
#define key_f7              NULL
#define key_f8              NULL
#define key_f9              NULL
#define key_f10             NULL
#define key_f11             NULL
#define key_f12             NULL
#define key_npage           NULL
#define key_ppage           NULL

/* Terminal structure - stub */
typedef struct _terminal {
    char *term_names;
    short Filedes;
    /* Add more fields as needed */
} TERMINAL;

/* cur_term is used by tty-term.c */
extern TERMINAL *cur_term;

/* Functions - stubs */
static __inline int setupterm(const char *term, int filedes, int *errret)
{
    (void)term; (void)filedes;
    if (errret) *errret = 1;
    return 0;
}

static __inline int del_curterm(TERMINAL *termp)
{
    (void)termp;
    return 0;
}

static __inline int putp(const char *str)
{
    (void)str;
    return 0;
}

static __inline char *tigetstr(const char *capname)
{
    (void)capname;
    return NULL;
}

static __inline int tigetnum(const char *capname)
{
    (void)capname;
    return -1;
}

static __inline int tigetflag(const char *capname)
{
    (void)capname;
    return 0;
}

static __inline char *tparm(const char *str, ...)
{
    (void)str;
    return NULL;
}

#endif /* _WIN32 */
#endif /* COMPAT_TERM_H */
