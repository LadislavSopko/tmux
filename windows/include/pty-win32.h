/*
 * pty-win32.h - ConPTY wrapper interface for Windows
 *
 * Provides pseudo-terminal functionality using Windows ConPTY API.
 */

#ifndef PTY_WIN32_H
#define PTY_WIN32_H

#ifdef _WIN32

#include <sys/types.h>

/* Forward declaration */
struct pty_handle;
typedef struct pty_handle pty_handle_t;

/*
 * Create a new pseudo-terminal with specified dimensions.
 * Returns a pty_handle_t pointer on success, NULL on failure.
 */
pty_handle_t *pty_create(int cols, int rows);

/*
 * Spawn a process in the pseudo-terminal.
 * Returns the process ID on success, -1 on failure.
 *
 * cmd  - Command line to execute (e.g., "cmd.exe" or "powershell.exe")
 * argv - Argument vector (currently unused, cmd contains full command)
 * envp - Environment variables (NULL-terminated array of "KEY=VALUE" strings)
 */
pid_t pty_spawn(pty_handle_t *pty, const char *cmd, char **argv, char **envp);

/*
 * Read from the pseudo-terminal (non-blocking).
 * Returns number of bytes read, 0 if no data available, -1 on error.
 */
ssize_t pty_read(pty_handle_t *pty, void *buf, size_t len);

/*
 * Write to the pseudo-terminal.
 * Returns number of bytes written, -1 on error.
 */
ssize_t pty_write(pty_handle_t *pty, const void *buf, size_t len);

/*
 * Resize the pseudo-terminal.
 * Returns 0 on success, -1 on failure.
 */
int pty_resize(pty_handle_t *pty, int cols, int rows);

/*
 * Destroy the pseudo-terminal and free all resources.
 */
void pty_destroy(pty_handle_t *pty);

/*
 * Get file descriptor for the PTY (for use with select/poll/libevent).
 * Returns the read fd (PTY output -> caller).
 */
int pty_get_fd(pty_handle_t *pty);

/*
 * Get the child process ID.
 */
pid_t pty_get_process_id(pty_handle_t *pty);

/*
 * Check if the child process is still running.
 * Returns 1 if alive, 0 if terminated.
 */
int pty_is_alive(pty_handle_t *pty);

#endif /* _WIN32 */
#endif /* PTY_WIN32_H */
