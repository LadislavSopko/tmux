/* pty-win32.c - ConPTY wrapper for Windows */

#include "compat-win32.h"
#include "tmux.h"

/*
 * fdforkpty - fork with pseudo-terminal
 *
 * On Unix, this forks the process and creates a pty. On Windows,
 * we use CreateProcess with ConPTY instead - there is no fork().
 *
 * This stub returns -1 to indicate "use alternative path".
 * The actual process spawning is handled by job_run() which
 * will need to be modified to use ConPTY directly.
 *
 * TODO: Phase 3 - Implement proper ConPTY spawning in job_run
 */
pid_t
fdforkpty(int ptmfd, int *master, char *name, struct termios *tio,
    struct winsize *ws)
{
    (void)ptmfd;
    (void)master;
    (void)name;
    (void)tio;
    (void)ws;

    /* Return -1 to indicate error - caller should handle Windows path */
    return (pid_t)-1;
}

/*
 * PTY handle structure for ConPTY
 * TODO: Phase 3 - Full ConPTY implementation
 */
struct pty_handle {
    HPCON hPC;           /* Pseudo console handle */
    HANDLE hPipeIn;      /* Pipe for reading from ConPTY */
    HANDLE hPipeOut;     /* Pipe for writing to ConPTY */
    HANDLE hProcess;     /* Child process handle */
    DWORD dwProcessId;   /* Child process ID */
};

/*
 * Stub implementations for PTY functions
 * TODO: Phase 3 - Full ConPTY implementation based on POC-01
 */

pty_handle_t *
pty_create(int cols, int rows)
{
    (void)cols;
    (void)rows;
    /* TODO: Implement with CreatePseudoConsole */
    return NULL;
}

pid_t
pty_spawn(pty_handle_t *pty, const char *cmd, char **argv, char **env)
{
    (void)pty;
    (void)cmd;
    (void)argv;
    (void)env;
    /* TODO: Implement with CreateProcess + PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE */
    return (pid_t)-1;
}

int
pty_resize(pty_handle_t *pty, int cols, int rows)
{
    (void)pty;
    (void)cols;
    (void)rows;
    /* TODO: Implement with ResizePseudoConsole */
    return -1;
}

ssize_t
pty_read(pty_handle_t *pty, void *buf, size_t len)
{
    (void)pty;
    (void)buf;
    (void)len;
    /* TODO: Implement with ReadFile on hPipeIn */
    return -1;
}

ssize_t
pty_write(pty_handle_t *pty, const void *buf, size_t len)
{
    (void)pty;
    (void)buf;
    (void)len;
    /* TODO: Implement with WriteFile on hPipeOut */
    return -1;
}

void
pty_destroy(pty_handle_t *pty)
{
    (void)pty;
    /* TODO: Close all handles and free memory */
}

int
pty_get_fd(pty_handle_t *pty)
{
    (void)pty;
    /* TODO: Return a file descriptor for event loop integration */
    return -1;
}
