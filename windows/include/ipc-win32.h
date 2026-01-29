/*
 * ipc-win32.h - Named Pipes IPC interface for Windows
 *
 * Provides IPC functionality using Windows Named Pipes as a replacement
 * for Unix Domain Sockets.
 */

#ifndef IPC_WIN32_H
#define IPC_WIN32_H

#ifdef _WIN32

#include <sys/types.h>

/*
 * Convert Unix socket path to Windows Named Pipe path.
 * Unix: /tmp/tmux-1000/default
 * Windows: \\.\pipe\tmux-1000-default
 *
 * Returns allocated string, caller must free.
 */
char *ipc_socket_to_pipe_path(const char *socket_path);

/*
 * Create a Named Pipe server (replacement for socket+bind+listen).
 * Returns a file descriptor on success, -1 on failure.
 *
 * The returned fd can be used with libevent for async accept.
 */
int ipc_server_create(const char *path, char **cause);

/*
 * Accept a connection on a Named Pipe server.
 * Returns a file descriptor for the client connection, -1 on failure.
 *
 * After accepting, the server pipe is closed and a new instance is
 * created automatically for the next client (handled internally).
 */
int ipc_server_accept(int server_fd);

/*
 * Connect to a Named Pipe server (replacement for socket+connect).
 * Returns a file descriptor on success, -1 on failure.
 */
int ipc_client_connect(const char *path);

/*
 * Close an IPC connection and clean up associated handles.
 */
void ipc_close(int fd);

/*
 * Check if a Named Pipe server exists at the given path.
 * Returns 1 if exists, 0 if not.
 */
int ipc_server_exists(const char *path);

/*
 * Get the underlying HANDLE for a file descriptor.
 * Returns INVALID_HANDLE_VALUE on failure.
 */
void *ipc_get_handle(int fd);

#endif /* _WIN32 */
#endif /* IPC_WIN32_H */
