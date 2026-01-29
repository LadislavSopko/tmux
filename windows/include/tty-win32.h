/*
 * tty-win32.h - Windows terminal interface
 *
 * Provides terminal control functions using Windows Console API.
 */

#ifndef TTY_WIN32_H
#define TTY_WIN32_H

#ifdef _WIN32

#include <sys/types.h>

/*
 * Initialize Windows terminal support.
 * Enables Virtual Terminal processing for ANSI escape sequences.
 * Returns 0 on success, -1 on failure.
 */
int tty_win32_init(void);

/*
 * Shutdown Windows terminal support.
 * Restores original console mode.
 */
void tty_win32_shutdown(void);

/*
 * Get the console size.
 * Returns 0 on success, -1 on failure.
 */
int tty_win32_get_size(int *cols, int *rows);

/*
 * Set raw mode (disable line buffering, echo, etc.)
 * Returns 0 on success, -1 on failure.
 */
int tty_win32_set_raw(void);

/*
 * Restore cooked mode.
 * Returns 0 on success, -1 on failure.
 */
int tty_win32_restore(void);

/*
 * Get the console input handle.
 */
void *tty_win32_get_input_handle(void);

/*
 * Get the console output handle.
 */
void *tty_win32_get_output_handle(void);

/*
 * Check if a file descriptor is a console/tty.
 * Returns 1 if tty, 0 if not.
 */
int tty_win32_isatty(int fd);

/*
 * Convert console mode to termios flags and vice versa.
 * These are used by the termios compatibility layer.
 */
unsigned long tty_win32_termios_to_input_mode(const struct termios *t);
unsigned long tty_win32_termios_to_output_mode(const struct termios *t);
void tty_win32_input_mode_to_termios(unsigned long mode, struct termios *t);
void tty_win32_output_mode_to_termios(unsigned long mode, struct termios *t);

#endif /* _WIN32 */
#endif /* TTY_WIN32_H */
