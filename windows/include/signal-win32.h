/*
 * signal-win32.h - Signal emulation interface for Windows
 *
 * Provides POSIX signal-like functionality using Windows console events
 * and thread-based child process monitoring.
 */

#ifndef SIGNAL_WIN32_H
#define SIGNAL_WIN32_H

#ifdef _WIN32

#include <sys/types.h>

/*
 * Signal mapping:
 * SIGINT   → CTRL_C_EVENT
 * SIGHUP   → CTRL_CLOSE_EVENT
 * SIGTERM  → CTRL_BREAK_EVENT / Manual
 * SIGCHLD  → Child process monitoring thread
 * SIGWINCH → Console size polling
 * SIGCONT  → Not applicable (no job control on Windows)
 * SIGUSR1/2 → Manual events
 */

/* Signal handler function type */
typedef void (*signal_handler_t)(int);

/*
 * Initialize the signal emulation system.
 * Must be called once at startup.
 */
int signal_init(void);

/*
 * Shutdown the signal emulation system.
 */
void signal_shutdown(void);

/*
 * Register a signal handler.
 * Returns 0 on success, -1 on failure.
 */
int signal_register(int sig, signal_handler_t handler);

/*
 * Unregister a signal handler.
 */
void signal_unregister(int sig);

/*
 * Send a signal to a process.
 * On Windows, this may use different mechanisms depending on the signal.
 * Returns 0 on success, -1 on failure.
 */
int signal_kill(pid_t pid, int sig);

/*
 * Add a child process to be monitored for exit (SIGCHLD).
 * When the process exits, the SIGCHLD handler will be called.
 */
int signal_watch_child(pid_t pid, void *handle);

/*
 * Remove a child process from monitoring.
 */
void signal_unwatch_child(pid_t pid);

/*
 * Check for pending signals and dispatch handlers.
 * Call this periodically if not using libevent integration.
 */
void signal_dispatch(void);

/*
 * Get the current console size.
 * Returns 0 on success, -1 on failure.
 */
int signal_get_console_size(int *cols, int *rows);

/*
 * Enable SIGWINCH detection (console resize monitoring).
 * Starts a background check that will fire SIGWINCH when size changes.
 */
void signal_enable_winch(void);

/*
 * Disable SIGWINCH detection.
 */
void signal_disable_winch(void);

#endif /* _WIN32 */
#endif /* SIGNAL_WIN32_H */
