/*
 * signal-win32.c - Signal emulation for Windows
 *
 * Implements POSIX signal-like functionality using Windows console events
 * and thread-based child process monitoring.
 *
 * Based on POC-04 (pocs/04-console-events/events_poc.c).
 */

#include "compat-win32.h"
#include "tmux.h"

#include <process.h>  /* _beginthreadex */

/*
 * Signal handler storage
 */
static signal_handler_t signal_handlers[NSIG];

/*
 * Console size tracking for SIGWINCH
 */
static int last_cols = 0;
static int last_rows = 0;
static int winch_enabled = 0;

/*
 * Child process monitoring
 */
#define MAX_WATCHED_CHILDREN 256

struct watched_child {
    pid_t pid;
    HANDLE handle;
    int active;
};

static struct watched_child watched_children[MAX_WATCHED_CHILDREN];
static CRITICAL_SECTION children_lock;
static int children_lock_init = 0;

/*
 * Pending signal flags (set by handlers, cleared by dispatch)
 */
static volatile int pending_signals[NSIG];

/*
 * Console control handler - handles Ctrl+C, Ctrl+Break, etc.
 */
static BOOL WINAPI
console_ctrl_handler(DWORD event)
{
    int sig = 0;

    switch (event) {
    case CTRL_C_EVENT:
        sig = SIGINT;
        log_debug("%s: CTRL_C_EVENT -> SIGINT", __func__);
        break;

    case CTRL_BREAK_EVENT:
        sig = SIGTERM;
        log_debug("%s: CTRL_BREAK_EVENT -> SIGTERM", __func__);
        break;

    case CTRL_CLOSE_EVENT:
        sig = SIGHUP;
        log_debug("%s: CTRL_CLOSE_EVENT -> SIGHUP", __func__);
        break;

    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        sig = SIGHUP;
        log_debug("%s: LOGOFF/SHUTDOWN -> SIGHUP", __func__);
        break;

    default:
        return FALSE;
    }

    if (sig != 0) {
        pending_signals[sig] = 1;
        /* Also call handler directly if registered */
        if (signal_handlers[sig] != NULL &&
            signal_handlers[sig] != SIG_IGN &&
            signal_handlers[sig] != SIG_DFL) {
            signal_handlers[sig](sig);
        }
        return TRUE;
    }

    return FALSE;
}

/*
 * Child monitor thread - monitors a single child process
 */
static unsigned __stdcall
child_monitor_thread(void *arg)
{
    struct watched_child *wc = (struct watched_child *)arg;
    DWORD result;
    DWORD exitCode;

    log_debug("%s: monitoring child PID %d", __func__, (int)wc->pid);

    /* Wait for child to exit or be cancelled */
    result = WaitForSingleObject(wc->handle, INFINITE);

    if (result == WAIT_OBJECT_0) {
        /* Child exited */
        GetExitCodeProcess(wc->handle, &exitCode);
        log_debug("%s: child PID %d exited with code %lu",
            __func__, (int)wc->pid, exitCode);

        /* Mark SIGCHLD pending */
        pending_signals[SIGCHLD] = 1;

        /* Call SIGCHLD handler if registered */
        if (signal_handlers[SIGCHLD] != NULL &&
            signal_handlers[SIGCHLD] != SIG_IGN &&
            signal_handlers[SIGCHLD] != SIG_DFL) {
            signal_handlers[SIGCHLD](SIGCHLD);
        }
    }

    /* Mark as inactive */
    EnterCriticalSection(&children_lock);
    wc->active = 0;
    wc->pid = 0;
    wc->handle = NULL;
    LeaveCriticalSection(&children_lock);

    return 0;
}

/*
 * Initialize the signal emulation system.
 */
int
signal_init(void)
{
    int i;

    log_debug("%s: initializing signal emulation", __func__);

    /* Initialize handler array */
    for (i = 0; i < NSIG; i++) {
        signal_handlers[i] = SIG_DFL;
        pending_signals[i] = 0;
    }

    /* Initialize children tracking */
    if (!children_lock_init) {
        InitializeCriticalSection(&children_lock);
        children_lock_init = 1;
    }
    for (i = 0; i < MAX_WATCHED_CHILDREN; i++) {
        watched_children[i].pid = 0;
        watched_children[i].handle = NULL;
        watched_children[i].active = 0;
    }

    /* Install console control handler */
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        log_debug("%s: SetConsoleCtrlHandler failed: %lu",
            __func__, GetLastError());
        return -1;
    }

    /* Get initial console size for SIGWINCH */
    signal_get_console_size(&last_cols, &last_rows);
    log_debug("%s: initial console size %dx%d", __func__, last_cols, last_rows);

    return 0;
}

/*
 * Shutdown the signal emulation system.
 */
void
signal_shutdown(void)
{
    int i;

    log_debug("%s: shutting down signal emulation", __func__);

    /* Remove console handler */
    SetConsoleCtrlHandler(console_ctrl_handler, FALSE);

    /* Terminate any remaining monitored children */
    if (children_lock_init) {
        EnterCriticalSection(&children_lock);
        for (i = 0; i < MAX_WATCHED_CHILDREN; i++) {
            if (watched_children[i].active && watched_children[i].handle) {
                /* Just close handle, don't terminate process */
                CloseHandle(watched_children[i].handle);
                watched_children[i].active = 0;
            }
        }
        LeaveCriticalSection(&children_lock);
        DeleteCriticalSection(&children_lock);
        children_lock_init = 0;
    }
}

/*
 * Register a signal handler.
 */
int
signal_register(int sig, signal_handler_t handler)
{
    if (sig < 0 || sig >= NSIG) {
        errno = EINVAL;
        return -1;
    }

    log_debug("%s: registering handler for signal %d", __func__, sig);
    signal_handlers[sig] = handler;
    return 0;
}

/*
 * Unregister a signal handler.
 */
void
signal_unregister(int sig)
{
    if (sig >= 0 && sig < NSIG) {
        signal_handlers[sig] = SIG_DFL;
    }
}

/*
 * Send a signal to a process.
 */
int
signal_kill(pid_t pid, int sig)
{
    HANDLE hProcess;
    BOOL result;

    log_debug("%s: sending signal %d to PID %d", __func__, sig, (int)pid);

    /* Special case: signal 0 just checks if process exists */
    if (sig == 0) {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (hProcess == NULL)
            return -1;
        CloseHandle(hProcess);
        return 0;
    }

    /* For SIGKILL and SIGTERM, terminate the process */
    if (sig == SIGKILL || sig == SIGTERM) {
        hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess == NULL) {
            errno = ESRCH;
            return -1;
        }
        result = TerminateProcess(hProcess, 128 + sig);
        CloseHandle(hProcess);
        if (!result) {
            errno = EPERM;
            return -1;
        }
        return 0;
    }

    /* For SIGINT, try to send Ctrl+C event */
    if (sig == SIGINT) {
        /* GenerateConsoleCtrlEvent only works for processes in same console */
        result = GenerateConsoleCtrlEvent(CTRL_C_EVENT, pid);
        if (!result) {
            /* Fall back to terminate */
            hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, 128 + sig);
                CloseHandle(hProcess);
            }
        }
        return 0;
    }

    /* For other signals, we can't really deliver them on Windows */
    /* Just check if process exists and return success */
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        errno = ESRCH;
        return -1;
    }
    CloseHandle(hProcess);
    return 0;
}

/*
 * Add a child process to be monitored for exit.
 */
int
signal_watch_child(pid_t pid, void *handle)
{
    int i, slot = -1;
    HANDLE hThread;
    HANDLE hDup;

    if (!children_lock_init) {
        signal_init();
    }

    /* Duplicate handle so we own it */
    if (!DuplicateHandle(GetCurrentProcess(), (HANDLE)handle,
                         GetCurrentProcess(), &hDup,
                         SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, 0)) {
        log_debug("%s: DuplicateHandle failed: %lu", __func__, GetLastError());
        return -1;
    }

    EnterCriticalSection(&children_lock);

    /* Find free slot */
    for (i = 0; i < MAX_WATCHED_CHILDREN; i++) {
        if (!watched_children[i].active) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        LeaveCriticalSection(&children_lock);
        CloseHandle(hDup);
        log_debug("%s: no free slots for child monitoring", __func__);
        return -1;
    }

    /* Fill slot */
    watched_children[slot].pid = pid;
    watched_children[slot].handle = hDup;
    watched_children[slot].active = 1;

    LeaveCriticalSection(&children_lock);

    /* Start monitor thread */
    hThread = (HANDLE)_beginthreadex(NULL, 0, child_monitor_thread,
                                     &watched_children[slot], 0, NULL);
    if (hThread == NULL) {
        EnterCriticalSection(&children_lock);
        watched_children[slot].active = 0;
        LeaveCriticalSection(&children_lock);
        CloseHandle(hDup);
        log_debug("%s: _beginthreadex failed", __func__);
        return -1;
    }
    CloseHandle(hThread);  /* Thread runs independently */

    log_debug("%s: watching child PID %d in slot %d", __func__, (int)pid, slot);
    return 0;
}

/*
 * Remove a child process from monitoring.
 */
void
signal_unwatch_child(pid_t pid)
{
    int i;

    if (!children_lock_init)
        return;

    EnterCriticalSection(&children_lock);
    for (i = 0; i < MAX_WATCHED_CHILDREN; i++) {
        if (watched_children[i].active && watched_children[i].pid == pid) {
            /* Can't really cancel the wait, but mark as inactive */
            watched_children[i].active = 0;
            log_debug("%s: unwatched child PID %d", __func__, (int)pid);
            break;
        }
    }
    LeaveCriticalSection(&children_lock);
}

/*
 * Check for pending signals and dispatch handlers.
 */
void
signal_dispatch(void)
{
    int i;

    for (i = 0; i < NSIG; i++) {
        if (pending_signals[i]) {
            pending_signals[i] = 0;
            if (signal_handlers[i] != NULL &&
                signal_handlers[i] != SIG_IGN &&
                signal_handlers[i] != SIG_DFL) {
                signal_handlers[i](i);
            }
        }
    }

    /* Check for console resize (SIGWINCH) */
    if (winch_enabled) {
        int cols, rows;
        if (signal_get_console_size(&cols, &rows) == 0) {
            if (cols != last_cols || rows != last_rows) {
                log_debug("%s: console resized %dx%d -> %dx%d",
                    __func__, last_cols, last_rows, cols, rows);
                last_cols = cols;
                last_rows = rows;
                if (signal_handlers[SIGWINCH] != NULL &&
                    signal_handlers[SIGWINCH] != SIG_IGN &&
                    signal_handlers[SIGWINCH] != SIG_DFL) {
                    signal_handlers[SIGWINCH](SIGWINCH);
                }
            }
        }
    }
}

/*
 * Get the current console size.
 */
int
signal_get_console_size(int *cols, int *rows)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hConsole == INVALID_HANDLE_VALUE)
        return -1;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        return -1;

    *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return 0;
}

/*
 * Enable SIGWINCH detection.
 */
void
signal_enable_winch(void)
{
    winch_enabled = 1;
    signal_get_console_size(&last_cols, &last_rows);
}

/*
 * Disable SIGWINCH detection.
 */
void
signal_disable_winch(void)
{
    winch_enabled = 0;
}

/*
 * POSIX sigaction() emulation - simplified version
 */
int
sigaction(int sig, const struct sigaction *act, struct sigaction *oldact)
{
    if (sig < 0 || sig >= NSIG) {
        errno = EINVAL;
        return -1;
    }

    if (oldact != NULL) {
        oldact->sa_handler = signal_handlers[sig];
        oldact->sa_flags = 0;
        sigemptyset(&oldact->sa_mask);
    }

    if (act != NULL) {
        signal_handlers[sig] = act->sa_handler;
    }

    return 0;
}

/*
 * POSIX kill() - implemented via signal_kill()
 */
int
kill(pid_t pid, int sig)
{
    return signal_kill(pid, sig);
}
