/*
 * sys/ioctl.h - Windows compatibility shim
 */

#ifndef _SYS_IOCTL_H_WIN32
#define _SYS_IOCTL_H_WIN32

#include <windows.h>
#include <stdarg.h>

/* ioctl requests */
#define TIOCGWINSZ  0x5413
#define TIOCSWINSZ  0x5414
#define TIOCNOTTY   0x5422
#define TIOCSCTTY   0x540E
#ifndef FIONREAD
#define FIONREAD    0x541B
#endif
#ifndef FIONBIO
#define FIONBIO     0x5421
#endif

/* Window size structure */
struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

/* ioctl stub */
static __inline int ioctl(int fd, unsigned long request, ...)
{
    (void)fd;

    if (request == TIOCGWINSZ) {
        /* Get console window size */
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        struct winsize *ws;
        va_list ap;
        va_start(ap, request);
        ws = va_arg(ap, struct winsize *);
        va_end(ap);

        if (ws && GetConsoleScreenBufferInfo(h, &csbi)) {
            ws->ws_col = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            ws->ws_row = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            ws->ws_xpixel = 0;
            ws->ws_ypixel = 0;
            return 0;
        }
        return -1;
    }

    return 0;
}

#endif /* _SYS_IOCTL_H_WIN32 */
