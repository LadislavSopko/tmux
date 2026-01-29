/*
 * pty-win32.c - ConPTY wrapper for Windows
 *
 * Implements pseudo-terminal functionality using Windows ConPTY API.
 * Based on POC-01 (pocs/01-conpty/conpty_poc.c).
 *
 * Target: Windows 10 1809+ (build 17763+)
 */

#include "compat-win32.h"
#include "tmux.h"

#include <io.h>      /* _open_osfhandle */
#include <fcntl.h>   /* O_RDWR */

/*
 * PTY handle structure for ConPTY
 */
struct pty_handle {
    HPCON hPC;              /* Pseudo console handle */
    HANDLE hPipeIn;         /* Pipe for reading from ConPTY (PTY -> us) */
    HANDLE hPipeOut;        /* Pipe for writing to ConPTY (us -> PTY) */
    HANDLE hProcess;        /* Child process handle */
    HANDLE hThread;         /* Child main thread handle */
    DWORD dwProcessId;      /* Child process ID */
    int fd_in;              /* File descriptor for hPipeIn (for libevent) */
    int fd_out;             /* File descriptor for hPipeOut */
};

/*
 * pty_create - Create a new pseudo-terminal
 *
 * Creates a ConPTY with the specified dimensions.
 * Returns a pty_handle_t pointer on success, NULL on failure.
 */
pty_handle_t *
pty_create(int cols, int rows)
{
    pty_handle_t *pty;
    HANDLE hPipePTYIn = NULL, hPipePTYOut = NULL;
    SECURITY_ATTRIBUTES sa = {0};
    COORD size;
    HRESULT hr;

    pty = xcalloc(1, sizeof(*pty));

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;  /* Pipes must be inheritable for child */

    /*
     * Create pipe pair for reading from PTY.
     * PTY stdout -> hPipePTYOut (write end, given to PTY)
     * Our stdin <- hPipeIn (read end, we read from)
     */
    if (!CreatePipe(&pty->hPipeIn, &hPipePTYOut, &sa, 0)) {
        log_debug("%s: CreatePipe (read) failed: %lu", __func__, GetLastError());
        goto fail;
    }

    /*
     * Create pipe pair for writing to PTY.
     * Our stdout -> hPipeOut (write end, we write to)
     * PTY stdin <- hPipePTYIn (read end, given to PTY)
     */
    if (!CreatePipe(&hPipePTYIn, &pty->hPipeOut, &sa, 0)) {
        log_debug("%s: CreatePipe (write) failed: %lu", __func__, GetLastError());
        goto fail;
    }

    /* Create pseudo console with specified dimensions */
    size.X = (SHORT)cols;
    size.Y = (SHORT)rows;

    hr = CreatePseudoConsole(size, hPipePTYIn, hPipePTYOut, 0, &pty->hPC);
    if (FAILED(hr)) {
        log_debug("%s: CreatePseudoConsole failed: 0x%lx", __func__, hr);
        goto fail;
    }

    /* Close PTY-side pipe handles (now owned by pseudo console) */
    CloseHandle(hPipePTYIn);
    CloseHandle(hPipePTYOut);

    /*
     * Convert pipe HANDLEs to file descriptors for libevent compatibility.
     * libevent can use select() with these fds on Windows.
     */
    pty->fd_in = _open_osfhandle((intptr_t)pty->hPipeIn, O_RDONLY);
    if (pty->fd_in == -1) {
        log_debug("%s: _open_osfhandle (in) failed", __func__);
        goto fail;
    }

    pty->fd_out = _open_osfhandle((intptr_t)pty->hPipeOut, O_WRONLY);
    if (pty->fd_out == -1) {
        log_debug("%s: _open_osfhandle (out) failed", __func__);
        /* Note: fd_in now owns hPipeIn, close() will close the handle */
        goto fail;
    }

    log_debug("%s: created ConPTY %dx%d, fd_in=%d, fd_out=%d",
        __func__, cols, rows, pty->fd_in, pty->fd_out);

    return pty;

fail:
    if (pty->hPC)
        ClosePseudoConsole(pty->hPC);
    if (hPipePTYIn)
        CloseHandle(hPipePTYIn);
    if (hPipePTYOut)
        CloseHandle(hPipePTYOut);
    if (pty->hPipeIn)
        CloseHandle(pty->hPipeIn);
    if (pty->hPipeOut)
        CloseHandle(pty->hPipeOut);
    free(pty);
    return NULL;
}

/*
 * pty_spawn - Spawn a process in the pseudo-terminal
 *
 * Creates a child process attached to the ConPTY.
 * Returns the process ID on success, -1 on failure.
 */
pid_t
pty_spawn(pty_handle_t *pty, const char *cmd, char **argv, char **envp)
{
    STARTUPINFOEXW si;
    PROCESS_INFORMATION pi;
    SIZE_T attrListSize = 0;
    LPPROC_THREAD_ATTRIBUTE_LIST attrList = NULL;
    wchar_t wcmdline[32768];  /* Max command line length */
    wchar_t *wenvblock = NULL;
    BOOL success = FALSE;
    size_t cmdlen;

    (void)argv;  /* TODO: build command line from argv if cmd is NULL */

    if (pty == NULL || pty->hPC == NULL) {
        log_debug("%s: invalid pty handle", __func__);
        return (pid_t)-1;
    }

    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(STARTUPINFOEXW);
    ZeroMemory(&pi, sizeof(pi));

    /* Convert command line to wide string */
    if (cmd == NULL)
        cmd = "cmd.exe";
    cmdlen = MultiByteToWideChar(CP_UTF8, 0, cmd, -1, wcmdline, 32768);
    if (cmdlen == 0) {
        log_debug("%s: MultiByteToWideChar failed: %lu", __func__, GetLastError());
        return (pid_t)-1;
    }

    /* Build environment block if provided */
    if (envp != NULL) {
        size_t total = 0;
        char **ep;
        wchar_t *wp;

        /* Calculate total size needed */
        for (ep = envp; *ep != NULL; ep++) {
            total += MultiByteToWideChar(CP_UTF8, 0, *ep, -1, NULL, 0);
        }
        total++;  /* Double null terminator */

        wenvblock = xcalloc(total, sizeof(wchar_t));
        wp = wenvblock;
        for (ep = envp; *ep != NULL; ep++) {
            int len = MultiByteToWideChar(CP_UTF8, 0, *ep, -1, wp, (int)(total - (wp - wenvblock)));
            wp += len;
        }
        *wp = L'\0';  /* Double null terminator */
    }

    /* Get required size for attribute list */
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);
    attrList = HeapAlloc(GetProcessHeap(), 0, attrListSize);
    if (attrList == NULL) {
        log_debug("%s: HeapAlloc failed", __func__);
        free(wenvblock);
        return (pid_t)-1;
    }

    /* Initialize attribute list */
    if (!InitializeProcThreadAttributeList(attrList, 1, 0, &attrListSize)) {
        log_debug("%s: InitializeProcThreadAttributeList failed: %lu",
            __func__, GetLastError());
        HeapFree(GetProcessHeap(), 0, attrList);
        free(wenvblock);
        return (pid_t)-1;
    }

    /* Set pseudo console attribute */
    if (!UpdateProcThreadAttribute(attrList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                   pty->hPC, sizeof(HPCON), NULL, NULL)) {
        log_debug("%s: UpdateProcThreadAttribute failed: %lu",
            __func__, GetLastError());
        DeleteProcThreadAttributeList(attrList);
        HeapFree(GetProcessHeap(), 0, attrList);
        free(wenvblock);
        return (pid_t)-1;
    }

    si.lpAttributeList = attrList;

    /* Create the process */
    success = CreateProcessW(
        NULL,                           /* lpApplicationName */
        wcmdline,                       /* lpCommandLine (modifiable) */
        NULL,                           /* lpProcessAttributes */
        NULL,                           /* lpThreadAttributes */
        FALSE,                          /* bInheritHandles */
        EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT,
        wenvblock,                      /* lpEnvironment */
        NULL,                           /* lpCurrentDirectory - TODO: support cwd */
        &si.StartupInfo,                /* lpStartupInfo */
        &pi                             /* lpProcessInformation */
    );

    DeleteProcThreadAttributeList(attrList);
    HeapFree(GetProcessHeap(), 0, attrList);
    free(wenvblock);

    if (!success) {
        log_debug("%s: CreateProcessW failed: %lu", __func__, GetLastError());
        return (pid_t)-1;
    }

    pty->hProcess = pi.hProcess;
    pty->hThread = pi.hThread;
    pty->dwProcessId = pi.dwProcessId;

    log_debug("%s: spawned process PID=%lu, cmd=%s",
        __func__, pty->dwProcessId, cmd);

    return (pid_t)pty->dwProcessId;
}

/*
 * pty_read - Read from the pseudo-terminal
 *
 * Reads data from the PTY output pipe.
 * Returns number of bytes read, 0 if no data available, -1 on error.
 */
ssize_t
pty_read(pty_handle_t *pty, void *buf, size_t len)
{
    DWORD available = 0;
    DWORD bytesRead = 0;

    if (pty == NULL || pty->hPipeIn == NULL)
        return -1;

    /* Check if data available (non-blocking peek) */
    if (!PeekNamedPipe(pty->hPipeIn, NULL, 0, NULL, &available, NULL)) {
        /* Pipe might be closed */
        DWORD err = GetLastError();
        if (err == ERROR_BROKEN_PIPE)
            return 0;  /* EOF */
        log_debug("%s: PeekNamedPipe failed: %lu", __func__, err);
        return -1;
    }

    if (available == 0)
        return 0;  /* No data available */

    /* Read available data */
    if (!ReadFile(pty->hPipeIn, buf, (DWORD)len, &bytesRead, NULL)) {
        DWORD err = GetLastError();
        if (err == ERROR_BROKEN_PIPE)
            return 0;  /* EOF */
        log_debug("%s: ReadFile failed: %lu", __func__, err);
        return -1;
    }

    return (ssize_t)bytesRead;
}

/*
 * pty_write - Write to the pseudo-terminal
 *
 * Writes data to the PTY input pipe.
 * Returns number of bytes written, -1 on error.
 */
ssize_t
pty_write(pty_handle_t *pty, const void *buf, size_t len)
{
    DWORD written = 0;

    if (pty == NULL || pty->hPipeOut == NULL)
        return -1;

    if (!WriteFile(pty->hPipeOut, buf, (DWORD)len, &written, NULL)) {
        log_debug("%s: WriteFile failed: %lu", __func__, GetLastError());
        return -1;
    }

    return (ssize_t)written;
}

/*
 * pty_resize - Resize the pseudo-terminal
 *
 * Changes the dimensions of the ConPTY.
 * Returns 0 on success, -1 on failure.
 */
int
pty_resize(pty_handle_t *pty, int cols, int rows)
{
    COORD size;
    HRESULT hr;

    if (pty == NULL || pty->hPC == NULL)
        return -1;

    size.X = (SHORT)cols;
    size.Y = (SHORT)rows;

    hr = ResizePseudoConsole(pty->hPC, size);
    if (FAILED(hr)) {
        log_debug("%s: ResizePseudoConsole failed: 0x%lx", __func__, hr);
        return -1;
    }

    log_debug("%s: resized to %dx%d", __func__, cols, rows);
    return 0;
}

/*
 * pty_destroy - Destroy the pseudo-terminal
 *
 * Closes all handles and frees the pty_handle_t.
 */
void
pty_destroy(pty_handle_t *pty)
{
    if (pty == NULL)
        return;

    log_debug("%s: destroying PTY, PID=%lu",
        __func__, pty->dwProcessId);

    /* Close pseudo console first */
    if (pty->hPC) {
        ClosePseudoConsole(pty->hPC);
        pty->hPC = NULL;
    }

    /* Terminate and close process handles */
    if (pty->hProcess) {
        TerminateProcess(pty->hProcess, 0);
        CloseHandle(pty->hProcess);
        pty->hProcess = NULL;
    }
    if (pty->hThread) {
        CloseHandle(pty->hThread);
        pty->hThread = NULL;
    }

    /*
     * Close file descriptors (which also close the underlying handles).
     * Only close if the fd was successfully created.
     */
    if (pty->fd_in != -1 && pty->fd_in != 0) {
        _close(pty->fd_in);
        pty->fd_in = -1;
        pty->hPipeIn = NULL;  /* Handle now invalid */
    } else if (pty->hPipeIn) {
        CloseHandle(pty->hPipeIn);
        pty->hPipeIn = NULL;
    }

    if (pty->fd_out != -1 && pty->fd_out != 0) {
        _close(pty->fd_out);
        pty->fd_out = -1;
        pty->hPipeOut = NULL;
    } else if (pty->hPipeOut) {
        CloseHandle(pty->hPipeOut);
        pty->hPipeOut = NULL;
    }

    free(pty);
}

/*
 * pty_get_fd - Get file descriptor for the PTY
 *
 * Returns a file descriptor that can be used with select/poll/libevent.
 * This is the read fd (PTY output -> us).
 */
int
pty_get_fd(pty_handle_t *pty)
{
    if (pty == NULL)
        return -1;
    return pty->fd_in;
}

/*
 * pty_get_process_id - Get the child process ID
 */
pid_t
pty_get_process_id(pty_handle_t *pty)
{
    if (pty == NULL)
        return (pid_t)-1;
    return (pid_t)pty->dwProcessId;
}

/*
 * pty_is_alive - Check if the child process is still running
 */
int
pty_is_alive(pty_handle_t *pty)
{
    DWORD exitCode;

    if (pty == NULL || pty->hProcess == NULL)
        return 0;

    if (!GetExitCodeProcess(pty->hProcess, &exitCode))
        return 0;

    return (exitCode == STILL_ACTIVE);
}

/*
 * fdforkpty - POSIX compatibility wrapper
 *
 * On Unix, fdforkpty forks the process and returns:
 * - In parent: child PID, *master set to pty master fd
 * - In child: 0
 * - On error: -1
 *
 * On Windows, there is no fork(). This function creates a ConPTY
 * and spawns the shell process directly. It always returns the
 * child PID (parent path), never 0 (child path).
 *
 * The caller (spawn.c, job.c) needs to be modified to handle
 * the Windows case where we don't enter the "child" code path.
 *
 * For now, this is a stub that returns -1. The actual PTY creation
 * and process spawning is done separately via pty_create/pty_spawn.
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

    /*
     * TODO: This needs integration work in spawn.c and job.c.
     *
     * On Windows, we need to:
     * 1. Create ConPTY with pty_create()
     * 2. Spawn process with pty_spawn()
     * 3. Return PID and set *master to pty_get_fd()
     *
     * But the caller expects to handle the "child" case (pid == 0)
     * which doesn't exist on Windows. For now, return -1 to indicate
     * that the standard path failed, and let the caller handle it.
     */

    log_debug("%s: called (Windows stub)", __func__);
    errno = ENOSYS;
    return (pid_t)-1;
}
