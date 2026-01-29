/*
 * ipc-win32.c - Named Pipes IPC for Windows
 *
 * Implements IPC using Windows Named Pipes as a replacement for
 * Unix Domain Sockets. Based on POC-02 (pocs/02-named-pipes/).
 *
 * Target: Windows 10+
 */

#include "compat-win32.h"
#include "tmux.h"

#include <io.h>      /* _open_osfhandle, _close */
#include <fcntl.h>   /* O_RDWR */

#define IPC_PIPE_PREFIX "\\\\.\\pipe\\"
#define IPC_BUFFER_SIZE 65536

/*
 * Internal structure to track pipe state.
 * We maintain a mapping from fd to HANDLE and pipe metadata.
 */
struct ipc_pipe {
    HANDLE hPipe;
    HANDLE hEvent;      /* For overlapped I/O */
    char *path;         /* Pipe path for recreating instances */
    int is_server;      /* True if this is a server pipe */
    int connected;      /* True if client is connected */
};

/* Simple fd -> ipc_pipe mapping (limited to reasonable number) */
#define MAX_IPC_PIPES 256
static struct ipc_pipe *ipc_pipes[MAX_IPC_PIPES];

static struct ipc_pipe *
ipc_get_pipe(int fd)
{
    if (fd < 0 || fd >= MAX_IPC_PIPES)
        return NULL;
    return ipc_pipes[fd];
}

static void
ipc_set_pipe(int fd, struct ipc_pipe *pipe)
{
    if (fd >= 0 && fd < MAX_IPC_PIPES)
        ipc_pipes[fd] = pipe;
}

/*
 * Convert Unix socket path to Windows Named Pipe path.
 * Unix: /tmp/tmux-1000/default
 * Windows: \\.\pipe\tmux-1000-default
 */
char *
ipc_socket_to_pipe_path(const char *socket_path)
{
    char *result;
    char *p;
    const char *name;
    size_t len;

    if (socket_path == NULL)
        return NULL;

    /* Find the basename or use full path if simple */
    name = strrchr(socket_path, '/');
    if (name == NULL)
        name = strrchr(socket_path, '\\');
    if (name != NULL)
        name++;  /* Skip separator */
    else
        name = socket_path;

    /* Build pipe path: \\.\pipe\tmux-<name> */
    len = strlen(IPC_PIPE_PREFIX) + 5 + strlen(name) + 1;
    result = xmalloc(len);
    snprintf(result, len, "%stmux-%s", IPC_PIPE_PREFIX, name);

    /* Replace any remaining path separators with dashes */
    for (p = result + strlen(IPC_PIPE_PREFIX); *p; p++) {
        if (*p == '/' || *p == '\\')
            *p = '-';
    }

    return result;
}

/*
 * Create a Named Pipe server.
 * Returns a file descriptor on success, -1 on failure.
 */
int
ipc_server_create(const char *path, char **cause)
{
    HANDLE hPipe;
    HANDLE hEvent;
    struct ipc_pipe *pipe;
    char *pipe_path;
    int fd;

    pipe_path = ipc_socket_to_pipe_path(path);
    if (pipe_path == NULL) {
        if (cause != NULL)
            xasprintf(cause, "invalid socket path: %s", path);
        return -1;
    }

    log_debug("%s: creating pipe %s", __func__, pipe_path);

    /* Create event for overlapped I/O */
    hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL) {
        if (cause != NULL)
            xasprintf(cause, "CreateEvent failed: %lu", GetLastError());
        free(pipe_path);
        return -1;
    }

    /* Create the named pipe */
    hPipe = CreateNamedPipeA(
        pipe_path,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        IPC_BUFFER_SIZE,
        IPC_BUFFER_SIZE,
        0,
        NULL  /* TODO: Security attributes for access control */
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (cause != NULL) {
            if (err == ERROR_PIPE_BUSY || err == ERROR_ACCESS_DENIED)
                xasprintf(cause, "pipe already exists: %s", pipe_path);
            else
                xasprintf(cause, "CreateNamedPipe failed (%lu): %s", err, pipe_path);
        }
        CloseHandle(hEvent);
        free(pipe_path);
        return -1;
    }

    /* Convert to file descriptor for libevent compatibility */
    fd = _open_osfhandle((intptr_t)hPipe, O_RDWR);
    if (fd == -1) {
        if (cause != NULL)
            xasprintf(cause, "_open_osfhandle failed");
        CloseHandle(hPipe);
        CloseHandle(hEvent);
        free(pipe_path);
        return -1;
    }

    /* Store pipe metadata */
    pipe = xcalloc(1, sizeof(*pipe));
    pipe->hPipe = hPipe;
    pipe->hEvent = hEvent;
    pipe->path = pipe_path;
    pipe->is_server = 1;
    pipe->connected = 0;
    ipc_set_pipe(fd, pipe);

    log_debug("%s: created server pipe fd=%d, path=%s", __func__, fd, pipe_path);
    return fd;
}

/*
 * Accept a connection on a Named Pipe server.
 * Returns a file descriptor for the client connection, -1 on failure.
 */
int
ipc_server_accept(int server_fd)
{
    struct ipc_pipe *server_pipe, *client_pipe;
    HANDLE hNewPipe, hEvent;
    OVERLAPPED ov = {0};
    DWORD err;
    int client_fd;

    server_pipe = ipc_get_pipe(server_fd);
    if (server_pipe == NULL || !server_pipe->is_server) {
        log_debug("%s: invalid server fd %d", __func__, server_fd);
        errno = EBADF;
        return -1;
    }

    /* If not yet connected, wait for connection */
    if (!server_pipe->connected) {
        ov.hEvent = server_pipe->hEvent;

        if (!ConnectNamedPipe(server_pipe->hPipe, &ov)) {
            err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                /* Wait for connection (this blocks - for async, use libevent) */
                WaitForSingleObject(server_pipe->hEvent, INFINITE);
            } else if (err != ERROR_PIPE_CONNECTED) {
                log_debug("%s: ConnectNamedPipe failed: %lu", __func__, err);
                errno = ECONNABORTED;
                return -1;
            }
        }
        server_pipe->connected = 1;
    }

    /*
     * On Windows Named Pipes, the "accept" model is different from sockets.
     * The current pipe handle IS the client connection.
     * We need to:
     * 1. Return the current pipe as the client fd
     * 2. Create a new pipe instance for the next client
     */

    /* Create new event for the new server instance */
    hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL) {
        log_debug("%s: CreateEvent failed: %lu", __func__, GetLastError());
        errno = ENOMEM;
        return -1;
    }

    /* Create new pipe instance for next client */
    hNewPipe = CreateNamedPipeA(
        server_pipe->path,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        IPC_BUFFER_SIZE,
        IPC_BUFFER_SIZE,
        0,
        NULL
    );

    if (hNewPipe == INVALID_HANDLE_VALUE) {
        log_debug("%s: CreateNamedPipe (new instance) failed: %lu",
            __func__, GetLastError());
        CloseHandle(hEvent);
        /* Still return the client connection, just can't accept more */
    }

    /* The current server_fd becomes the client fd */
    client_fd = server_fd;
    client_pipe = server_pipe;
    client_pipe->is_server = 0;  /* Now it's a client connection */

    /* If we created a new pipe, set up new server fd */
    if (hNewPipe != INVALID_HANDLE_VALUE) {
        int new_server_fd;
        struct ipc_pipe *new_server_pipe;
        HANDLE oldEvent = client_pipe->hEvent;
        char *path_copy = xstrdup(client_pipe->path);

        new_server_fd = _open_osfhandle((intptr_t)hNewPipe, O_RDWR);
        if (new_server_fd != -1) {
            new_server_pipe = xcalloc(1, sizeof(*new_server_pipe));
            new_server_pipe->hPipe = hNewPipe;
            new_server_pipe->hEvent = hEvent;
            new_server_pipe->path = path_copy;
            new_server_pipe->is_server = 1;
            new_server_pipe->connected = 0;
            ipc_set_pipe(new_server_fd, new_server_pipe);

            /* Update server_fd global - caller needs to handle this */
            /* For now, we just log it */
            log_debug("%s: new server instance fd=%d", __func__, new_server_fd);
        } else {
            CloseHandle(hNewPipe);
            CloseHandle(hEvent);
            free(path_copy);
        }

        /* Client keeps old event for now, will be cleaned on close */
        client_pipe->hEvent = oldEvent;
    }

    log_debug("%s: accepted client fd=%d", __func__, client_fd);
    return client_fd;
}

/*
 * Connect to a Named Pipe server.
 * Returns a file descriptor on success, -1 on failure.
 */
int
ipc_client_connect(const char *path)
{
    HANDLE hPipe;
    struct ipc_pipe *pipe;
    char *pipe_path;
    int fd;
    DWORD mode;

    pipe_path = ipc_socket_to_pipe_path(path);
    if (pipe_path == NULL) {
        errno = EINVAL;
        return -1;
    }

    log_debug("%s: connecting to %s", __func__, pipe_path);

    /* Try to connect to the pipe */
    hPipe = CreateFileA(
        pipe_path,
        GENERIC_READ | GENERIC_WRITE,
        0,              /* No sharing */
        NULL,           /* Default security */
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        log_debug("%s: CreateFile failed: %lu", __func__, err);

        if (err == ERROR_FILE_NOT_FOUND) {
            errno = ECONNREFUSED;  /* No server */
        } else if (err == ERROR_PIPE_BUSY) {
            /* Pipe is busy, could wait with WaitNamedPipe */
            errno = EAGAIN;
        } else {
            errno = EIO;
        }
        free(pipe_path);
        return -1;
    }

    /* Set pipe to byte mode */
    mode = PIPE_READMODE_BYTE;
    if (!SetNamedPipeHandleState(hPipe, &mode, NULL, NULL)) {
        log_debug("%s: SetNamedPipeHandleState failed: %lu",
            __func__, GetLastError());
        CloseHandle(hPipe);
        free(pipe_path);
        errno = EIO;
        return -1;
    }

    /* Convert to file descriptor */
    fd = _open_osfhandle((intptr_t)hPipe, O_RDWR);
    if (fd == -1) {
        CloseHandle(hPipe);
        free(pipe_path);
        return -1;
    }

    /* Store pipe metadata */
    pipe = xcalloc(1, sizeof(*pipe));
    pipe->hPipe = hPipe;
    pipe->hEvent = NULL;
    pipe->path = pipe_path;
    pipe->is_server = 0;
    pipe->connected = 1;
    ipc_set_pipe(fd, pipe);

    log_debug("%s: connected fd=%d, path=%s", __func__, fd, pipe_path);
    return fd;
}

/*
 * Close an IPC connection.
 */
void
ipc_close(int fd)
{
    struct ipc_pipe *pipe = ipc_get_pipe(fd);

    if (pipe != NULL) {
        log_debug("%s: closing fd=%d, path=%s", __func__, fd,
            pipe->path ? pipe->path : "(null)");

        if (pipe->is_server && pipe->connected) {
            DisconnectNamedPipe(pipe->hPipe);
        }
        if (pipe->hEvent)
            CloseHandle(pipe->hEvent);
        free(pipe->path);
        free(pipe);
        ipc_set_pipe(fd, NULL);
    }

    /* _close will close the underlying HANDLE via _open_osfhandle */
    _close(fd);
}

/*
 * Check if a Named Pipe server exists.
 */
int
ipc_server_exists(const char *path)
{
    HANDLE hPipe;
    char *pipe_path;
    int exists = 0;

    pipe_path = ipc_socket_to_pipe_path(path);
    if (pipe_path == NULL)
        return 0;

    /* Try to open the pipe */
    hPipe = CreateFileA(
        pipe_path,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe != INVALID_HANDLE_VALUE) {
        exists = 1;
        CloseHandle(hPipe);
    } else if (GetLastError() == ERROR_PIPE_BUSY) {
        /* Pipe exists but is busy */
        exists = 1;
    }

    free(pipe_path);
    return exists;
}

/*
 * Get the underlying HANDLE for a file descriptor.
 */
void *
ipc_get_handle(int fd)
{
    struct ipc_pipe *pipe = ipc_get_pipe(fd);
    if (pipe != NULL)
        return pipe->hPipe;
    return INVALID_HANDLE_VALUE;
}
