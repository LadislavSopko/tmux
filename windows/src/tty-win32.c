/*
 * tty-win32.c - Windows terminal implementation
 *
 * Provides terminal control using Windows Console API.
 * Enables Virtual Terminal processing for ANSI escape sequences.
 */

#include "compat-win32.h"
#include "tmux.h"

#include <io.h>

/* Console handles */
static HANDLE hConsoleInput = INVALID_HANDLE_VALUE;
static HANDLE hConsoleOutput = INVALID_HANDLE_VALUE;

/* Original console modes (for restoration) */
static DWORD dwOriginalInputMode = 0;
static DWORD dwOriginalOutputMode = 0;
static int modes_saved = 0;

/*
 * Initialize Windows terminal support.
 */
int
tty_win32_init(void)
{
    DWORD dwMode;

    log_debug("%s: initializing Windows terminal", __func__);

    /* Get console handles */
    hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hConsoleInput == INVALID_HANDLE_VALUE ||
        hConsoleOutput == INVALID_HANDLE_VALUE) {
        log_debug("%s: failed to get console handles", __func__);
        return -1;
    }

    /* Save original modes */
    if (!GetConsoleMode(hConsoleInput, &dwOriginalInputMode)) {
        log_debug("%s: GetConsoleMode (input) failed: %lu",
            __func__, GetLastError());
        return -1;
    }
    if (!GetConsoleMode(hConsoleOutput, &dwOriginalOutputMode)) {
        log_debug("%s: GetConsoleMode (output) failed: %lu",
            __func__, GetLastError());
        return -1;
    }
    modes_saved = 1;

    log_debug("%s: original input mode: 0x%lx, output mode: 0x%lx",
        __func__, dwOriginalInputMode, dwOriginalOutputMode);

    /*
     * Enable Virtual Terminal processing for ANSI escape sequences.
     * This is available on Windows 10 1607+ and allows tmux to use
     * standard ANSI sequences for terminal control.
     */
    dwMode = dwOriginalOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hConsoleOutput, dwMode)) {
        log_debug("%s: SetConsoleMode (VT output) failed: %lu",
            __func__, GetLastError());
        /* Continue anyway - basic functionality might work */
    }

    /*
     * Enable Virtual Terminal Input for proper input handling.
     */
    dwMode = dwOriginalInputMode | ENABLE_VIRTUAL_TERMINAL_INPUT;
    if (!SetConsoleMode(hConsoleInput, dwMode)) {
        log_debug("%s: SetConsoleMode (VT input) failed: %lu",
            __func__, GetLastError());
        /* Continue anyway */
    }

    log_debug("%s: Windows terminal initialized", __func__);
    return 0;
}

/*
 * Shutdown Windows terminal support.
 */
void
tty_win32_shutdown(void)
{
    log_debug("%s: shutting down Windows terminal", __func__);

    /* Restore original modes */
    if (modes_saved) {
        if (hConsoleInput != INVALID_HANDLE_VALUE)
            SetConsoleMode(hConsoleInput, dwOriginalInputMode);
        if (hConsoleOutput != INVALID_HANDLE_VALUE)
            SetConsoleMode(hConsoleOutput, dwOriginalOutputMode);
    }

    hConsoleInput = INVALID_HANDLE_VALUE;
    hConsoleOutput = INVALID_HANDLE_VALUE;
    modes_saved = 0;
}

/*
 * Get the console size.
 */
int
tty_win32_get_size(int *cols, int *rows)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (hConsoleOutput == INVALID_HANDLE_VALUE)
        return -1;

    if (!GetConsoleScreenBufferInfo(hConsoleOutput, &csbi))
        return -1;

    *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return 0;
}

/*
 * Set raw mode.
 */
int
tty_win32_set_raw(void)
{
    DWORD dwInputMode, dwOutputMode;

    if (hConsoleInput == INVALID_HANDLE_VALUE ||
        hConsoleOutput == INVALID_HANDLE_VALUE)
        return -1;

    /*
     * Raw input mode:
     * - Disable line input (ENABLE_LINE_INPUT)
     * - Disable echo (ENABLE_ECHO_INPUT)
     * - Disable processed input (ENABLE_PROCESSED_INPUT)
     * - Enable window input for resize events (ENABLE_WINDOW_INPUT)
     * - Enable Virtual Terminal input (ENABLE_VIRTUAL_TERMINAL_INPUT)
     */
    dwInputMode = ENABLE_WINDOW_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT;

    if (!SetConsoleMode(hConsoleInput, dwInputMode)) {
        log_debug("%s: SetConsoleMode (input) failed: %lu",
            __func__, GetLastError());
        return -1;
    }

    /*
     * Raw output mode:
     * - Enable Virtual Terminal processing (ENABLE_VIRTUAL_TERMINAL_PROCESSING)
     * - Disable wrap at EOL for precise cursor control
     */
    dwOutputMode = ENABLE_VIRTUAL_TERMINAL_PROCESSING |
                   DISABLE_NEWLINE_AUTO_RETURN;

    if (!SetConsoleMode(hConsoleOutput, dwOutputMode)) {
        log_debug("%s: SetConsoleMode (output) failed: %lu",
            __func__, GetLastError());
        /* Try without DISABLE_NEWLINE_AUTO_RETURN */
        dwOutputMode = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hConsoleOutput, dwOutputMode);
    }

    log_debug("%s: raw mode set", __func__);
    return 0;
}

/*
 * Restore cooked mode.
 */
int
tty_win32_restore(void)
{
    if (!modes_saved)
        return -1;

    if (hConsoleInput != INVALID_HANDLE_VALUE)
        SetConsoleMode(hConsoleInput, dwOriginalInputMode);
    if (hConsoleOutput != INVALID_HANDLE_VALUE)
        SetConsoleMode(hConsoleOutput, dwOriginalOutputMode);

    log_debug("%s: cooked mode restored", __func__);
    return 0;
}

/*
 * Get the console input handle.
 */
void *
tty_win32_get_input_handle(void)
{
    return hConsoleInput;
}

/*
 * Get the console output handle.
 */
void *
tty_win32_get_output_handle(void)
{
    return hConsoleOutput;
}

/*
 * Check if a file descriptor is a console/tty.
 */
int
tty_win32_isatty(int fd)
{
    HANDLE h;
    DWORD mode;

    h = (HANDLE)_get_osfhandle(fd);
    if (h == INVALID_HANDLE_VALUE)
        return 0;

    /* Try to get console mode - if it succeeds, it's a console */
    return GetConsoleMode(h, &mode) != 0;
}

/*
 * Convert termios flags to Windows input mode.
 */
unsigned long
tty_win32_termios_to_input_mode(const struct termios *t)
{
    DWORD mode = 0;

    if (t == NULL)
        return ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;

    /* Line input (ICANON) */
    if (t->c_lflag & ICANON)
        mode |= ENABLE_LINE_INPUT;

    /* Echo (ECHO) */
    if (t->c_lflag & ECHO)
        mode |= ENABLE_ECHO_INPUT;

    /* Signal processing (ISIG) */
    if (t->c_lflag & ISIG)
        mode |= ENABLE_PROCESSED_INPUT;

    /* Always enable window input for resize events */
    mode |= ENABLE_WINDOW_INPUT;

    /* Enable VT input for escape sequences */
    mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;

    return mode;
}

/*
 * Convert termios flags to Windows output mode.
 */
unsigned long
tty_win32_termios_to_output_mode(const struct termios *t)
{
    DWORD mode = ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (t == NULL)
        return mode | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;

    /* Post-processing (OPOST) */
    if (t->c_oflag & OPOST)
        mode |= ENABLE_PROCESSED_OUTPUT;

    return mode;
}

/*
 * Convert Windows input mode to termios flags.
 */
void
tty_win32_input_mode_to_termios(unsigned long mode, struct termios *t)
{
    if (t == NULL)
        return;

    t->c_lflag = 0;

    if (mode & ENABLE_LINE_INPUT)
        t->c_lflag |= ICANON;

    if (mode & ENABLE_ECHO_INPUT)
        t->c_lflag |= ECHO;

    if (mode & ENABLE_PROCESSED_INPUT)
        t->c_lflag |= ISIG;
}

/*
 * Convert Windows output mode to termios flags.
 */
void
tty_win32_output_mode_to_termios(unsigned long mode, struct termios *t)
{
    if (t == NULL)
        return;

    t->c_oflag = 0;

    if (mode & ENABLE_PROCESSED_OUTPUT)
        t->c_oflag |= OPOST;
}

/*
 * Implementation of tcgetattr for Windows.
 * This replaces the inline stub in termios.h.
 */
int
tcgetattr_win32(int fd, struct termios *termios_p)
{
    HANDLE h;
    DWORD dwInputMode, dwOutputMode;

    if (termios_p == NULL) {
        errno = EINVAL;
        return -1;
    }

    h = (HANDLE)_get_osfhandle(fd);
    if (h == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }

    /* Initialize with defaults */
    memset(termios_p, 0, sizeof(*termios_p));
    termios_p->c_cflag = CS8 | CREAD;
    termios_p->c_cc[VMIN] = 1;
    termios_p->c_cc[VTIME] = 0;

    /* Try to get input mode */
    if (GetConsoleMode(h, &dwInputMode)) {
        tty_win32_input_mode_to_termios(dwInputMode, termios_p);
    } else {
        /* Not a console, use defaults */
        termios_p->c_iflag = ICRNL | IXON;
        termios_p->c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK;
    }

    /* Try to get output mode from stdout */
    if (GetConsoleMode(hConsoleOutput, &dwOutputMode)) {
        tty_win32_output_mode_to_termios(dwOutputMode, termios_p);
    } else {
        termios_p->c_oflag = OPOST | ONLCR;
    }

    return 0;
}

/*
 * Implementation of tcsetattr for Windows.
 * This replaces the inline stub in termios.h.
 */
int
tcsetattr_win32(int fd, int optional_actions, const struct termios *termios_p)
{
    HANDLE h;
    DWORD dwInputMode, dwOutputMode;

    (void)optional_actions;  /* TCSANOW, TCSADRAIN, TCSAFLUSH all treated same */

    if (termios_p == NULL) {
        errno = EINVAL;
        return -1;
    }

    h = (HANDLE)_get_osfhandle(fd);
    if (h == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }

    /* Convert termios to Windows modes */
    dwInputMode = tty_win32_termios_to_input_mode(termios_p);
    dwOutputMode = tty_win32_termios_to_output_mode(termios_p);

    /* Set input mode */
    if (!SetConsoleMode(h, dwInputMode)) {
        /* Not a console or error - ignore */
    }

    /* Set output mode on stdout */
    if (hConsoleOutput != INVALID_HANDLE_VALUE) {
        SetConsoleMode(hConsoleOutput, dwOutputMode);
    }

    return 0;
}
