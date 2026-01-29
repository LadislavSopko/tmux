/*
 * compat-win32.h - Windows compatibility layer for tmux
 *
 * This header provides Windows equivalents for POSIX APIs and defines
 * used throughout the tmux codebase.
 */

#ifndef COMPAT_WIN32_H
#define COMPAT_WIN32_H

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Disable warnings for POSIX function names */
#pragma warning(disable: 4996)

/*
 * Path definitions - Windows equivalents
 */
#define _PATH_BSHELL    "cmd.exe"
#define _PATH_TMP       "C:\\Windows\\Temp"
#define _PATH_DEVNULL   "NUL"
#define _PATH_TTY       "CON"

/* tmux socket path template - uses named pipes */
#define TMUX_SOCK       "\\\\.\\pipe\\tmux-%s"

/*
 * File mode bits (not used on Windows but needed for compilation)
 */
#ifndef S_IRWXU
#define S_IRWXU  0700
#define S_IRUSR  0400
#define S_IWUSR  0200
#define S_IXUSR  0100
#define S_IRWXG  0070
#define S_IRGRP  0040
#define S_IWGRP  0020
#define S_IXGRP  0010
#define S_IRWXO  0007
#define S_IROTH  0004
#define S_IWOTH  0002
#define S_IXOTH  0001
#endif

#ifndef S_ISDIR
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)  (0)  /* No symlinks on Windows */
#define S_ISSOCK(m) (0)  /* No Unix sockets */
#define S_ISFIFO(m) (0)  /* No FIFOs */
#endif

/*
 * Process-related definitions
 */
typedef int pid_t;
typedef int uid_t;
typedef int gid_t;
typedef int mode_t;
typedef long ssize_t;

#define getpid()    _getpid()
#define getuid()    (0)  /* Windows doesn't have UIDs - use 0 for now */
#define getgid()    (0)
#define geteuid()   (0)
#define getegid()   (0)

/*
 * Signal definitions - map to Windows events
 * These are placeholders; actual implementation in signal-win32.c
 */
#ifndef SIGCHLD
#define SIGCHLD     17
#define SIGCONT     18
#define SIGSTOP     19
#define SIGTSTP     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGWINCH    28
#define SIGPIPE     13
#define SIGHUP      1
#define SIGINT      2
#define SIGQUIT     3
#define SIGTERM     15
#define SIGUSR1     10
#define SIGUSR2     12
#endif

#define SIG_DFL     ((void (*)(int))0)
#define SIG_IGN     ((void (*)(int))1)
#define SIG_ERR     ((void (*)(int))-1)

/*
 * Wait status macros
 */
#define WIFEXITED(status)   (1)
#define WEXITSTATUS(status) (status)
#define WIFSIGNALED(status) (0)
#define WTERMSIG(status)    (0)
#define WIFSTOPPED(status)  (0)
#define WSTOPSIG(status)    (0)

#define WNOHANG     1
#define WUNTRACED   2

/*
 * File descriptor operations
 */
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

#define F_GETFL     3
#define F_SETFL     4
#define F_GETFD     1
#define F_SETFD     2
#define FD_CLOEXEC  1
#define O_NONBLOCK  0x0004
#define O_CLOEXEC   0x80000

/*
 * Socket definitions - map to Named Pipes
 */
#define AF_UNIX     1
#define SOCK_STREAM 1
#define PF_UNSPEC   0

struct sockaddr_un {
    unsigned short sun_family;
    char sun_path[108];
};

/*
 * Function mappings
 */
#define chdir(p)        _chdir(p)
#define getcwd(b,s)     _getcwd(b,s)
#define mkdir(p,m)      _mkdir(p)
#define rmdir(p)        _rmdir(p)
#define unlink(p)       _unlink(p)
#define access(p,m)     _access(p,m)
#define open(p,f,...)   _open(p,f,__VA_ARGS__)
#define close(fd)       _close(fd)
#define read(fd,b,n)    _read(fd,b,(unsigned int)(n))
#define write(fd,b,n)   _write(fd,b,(unsigned int)(n))
#define lseek(fd,o,w)   _lseek(fd,o,w)
#define dup(fd)         _dup(fd)
#define dup2(fd1,fd2)   _dup2(fd1,fd2)
#define fileno(f)       _fileno(f)
#define isatty(fd)      _isatty(fd)
#define umask(m)        _umask(m)

#define R_OK    4
#define W_OK    2
#define X_OK    0  /* Windows doesn't have X_OK */
#define F_OK    0

/*
 * Time-related
 */
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

/*
 * Forward declarations for Windows implementations
 * (implemented in src/*.c)
 */

/* pty-win32.c */
struct pty_handle;
typedef struct pty_handle pty_handle_t;

pty_handle_t *pty_create(int cols, int rows);
pid_t pty_spawn(pty_handle_t *pty, const char *cmd, char **argv, char **env);
int pty_resize(pty_handle_t *pty, int cols, int rows);
ssize_t pty_read(pty_handle_t *pty, void *buf, size_t len);
ssize_t pty_write(pty_handle_t *pty, const void *buf, size_t len);
void pty_destroy(pty_handle_t *pty);
int pty_get_fd(pty_handle_t *pty);

/* ipc-win32.c */
struct ipc_handle;
typedef struct ipc_handle ipc_handle_t;

ipc_handle_t *ipc_listen(const char *path);
ipc_handle_t *ipc_connect(const char *path);
ipc_handle_t *ipc_accept(ipc_handle_t *server);
ssize_t ipc_send(ipc_handle_t *h, const void *buf, size_t len);
ssize_t ipc_recv(ipc_handle_t *h, void *buf, size_t len);
void ipc_close(ipc_handle_t *h);
int ipc_get_fd(ipc_handle_t *h);

/* proc-win32.c */
struct proc_handle;
typedef struct proc_handle proc_handle_t;

proc_handle_t *proc_spawn(const char *cmd, char **argv, char **env, const char *cwd);
int proc_wait(proc_handle_t *h, int *status, int flags);
int proc_kill(proc_handle_t *h, int signal);
void proc_close(proc_handle_t *h);
pid_t proc_get_pid(proc_handle_t *h);

/* signal-win32.c */
typedef void (*sighandler_t)(int);
sighandler_t win32_signal(int sig, sighandler_t handler);
int win32_kill(pid_t pid, int sig);
int win32_raise(int sig);
void win32_signal_init(void);

/* daemon-win32.c */
int win32_daemon(int nochdir, int noclose);

/* osdep-win32.c */
char *osdep_get_name(int fd, char *tty);
char *osdep_get_cwd(int fd);
struct event_base *osdep_event_init(void);

#endif /* _WIN32 */

#endif /* COMPAT_WIN32_H */
