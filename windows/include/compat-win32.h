/*
 * compat-win32.h - Windows compatibility layer for tmux
 *
 * This header provides Windows equivalents for POSIX APIs and defines
 * used throughout the tmux codebase.
 */

#ifndef COMPAT_WIN32_H
#define COMPAT_WIN32_H

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Windows defines 'environ' as a macro - we need to undef it
 * because tmux uses it as a struct member name.
 * We include stdlib.h above to ensure we have the real environ
 * definition before undefining the macro.
 * Note: We still need environ in some compat files (setenv.c),
 * but it conflicts with struct members in tmux code. We use
 * __environ as a workaround. */
#ifdef environ
#undef environ
#endif

/* Provide access to environment via __environ for compat code */
extern char **_environ;
#define __environ _environ

/* Also undefine 'entry' which conflicts with tmux macros */
#ifdef entry
#undef entry
#endif

/* Undefine SIZE which conflicts with tmux enum in popup.c */
#ifdef SIZE
#undef SIZE
#endif
#include <io.h>
#include <direct.h>
#include <process.h>
#include <fcntl.h>
#include <time.h>
#include <wchar.h>
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

/* PATH_MAX and NAME_MAX */
#ifndef PATH_MAX
#define PATH_MAX        260  /* MAX_PATH on Windows */
#endif
#ifndef NAME_MAX
#define NAME_MAX        255
#endif
#ifndef MAXPATHLEN
#define MAXPATHLEN      PATH_MAX
#endif

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

/* sigaction structure and flags */
#define SA_RESTART  0x10000000
#define SA_NOCLDSTOP 0x00000001
#define SA_NOCLDWAIT 0x00000002
#define SA_SIGINFO   0x00000004

/* sigprocmask 'how' values */
#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

typedef unsigned long sigset_t;

struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
};

static __inline int sigemptyset(sigset_t *set) { if (set) *set = 0; return 0; }
static __inline int sigfillset(sigset_t *set) { if (set) *set = ~0UL; return 0; }
static __inline int sigaddset(sigset_t *set, int signum) { if (set) *set |= (1UL << signum); return 0; }
static __inline int sigdelset(sigset_t *set, int signum) { if (set) *set &= ~(1UL << signum); return 0; }
static __inline int sigismember(const sigset_t *set, int signum) { return set ? ((*set >> signum) & 1) : 0; }

static __inline int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
    (void)sig; (void)act; (void)oact;
    /* Stub - actual implementation in signal-win32.c */
    return 0;
}

static __inline int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    (void)how; (void)set; (void)oset;
    return 0;
}

static __inline int kill(pid_t pid, int sig)
{
    (void)pid; (void)sig;
    /* Stub - actual implementation in signal-win32.c */
    return 0;
}

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
 * Note: sockaddr_un is defined in sys/un.h
 */
#ifndef AF_UNIX
#define AF_UNIX     1
#endif
#ifndef SOCK_STREAM
#define SOCK_STREAM 1
#endif

/* socketpair stub - Windows doesn't have this, use pipes instead */
static __inline int socketpair(int domain, int type, int protocol, int sv[2])
{
    (void)domain; (void)type; (void)protocol;
    /* Stub - actual implementation should use anonymous pipes or named pipes */
    if (sv) { sv[0] = -1; sv[1] = -1; }
    return -1;
}

/* shutdown() constants - Windows uses SD_* but we need SHUT_* */
#ifndef SHUT_RD
#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH
#endif

/* killpg stub - kill process group */
static __inline int killpg(pid_t pgrp, int sig)
{
    (void)pgrp; (void)sig;
    /* Stub - Windows doesn't have process groups like Unix */
    return -1;
}

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

/* String functions */
#define strcasecmp(s1,s2)     _stricmp(s1,s2)
#define strncasecmp(s1,s2,n)  _strnicmp(s1,s2,n)

/* ttyname stub */
static __inline char *ttyname(int fd) { (void)fd; return "CON"; }

/* strsignal stub */
static __inline const char *strsignal(int sig) {
    static char buf[32];
    sprintf(buf, "Signal %d", sig);
    return buf;
}

#define R_OK    4
#define W_OK    2
#define X_OK    0  /* Windows doesn't have X_OK */
#define F_OK    0

/*
 * File functions
 */

/* fseeko/ftello - 64-bit file position */
#define fseeko(f, o, w)  _fseeki64(f, o, w)
#define ftello(f)        _ftelli64(f)

/* mkstemp - create unique temporary file */
static __inline int mkstemp(char *tmpl)
{
    int fd = -1;
    if (_mktemp_s(tmpl, strlen(tmpl) + 1) == 0) {
        fd = _open(tmpl, _O_RDWR | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
    }
    return fd;
}

/* mkstemps - mkstemp with suffix */
static __inline int mkstemps(char *tmpl, int suffixlen)
{
    (void)suffixlen;
    return mkstemp(tmpl);
}

/*
 * Time-related functions
 */

/* gmtime_r - thread-safe gmtime */
static __inline struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
    struct tm *tmp = gmtime(timep);
    if (tmp && result) {
        *result = *tmp;
        return result;
    }
    return NULL;
}

/* localtime_r - thread-safe localtime */
static __inline struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    struct tm *tmp = localtime(timep);
    if (tmp && result) {
        *result = *tmp;
        return result;
    }
    return NULL;
}

/*
 * Wide character functions
 */

/* wcwidth - get display width of wide character */
static __inline int wcwidth(wchar_t wc)
{
    /* Basic implementation:
     * - 0 for NUL
     * - -1 for control characters
     * - 2 for wide characters (CJK, etc.)
     * - 1 for everything else
     */
    if (wc == 0)
        return 0;
    if (wc < 32 || (wc >= 0x7f && wc < 0xa0))
        return -1;
    /* CJK ranges - very simplified */
    if ((wc >= 0x1100 && wc <= 0x115f) ||   /* Hangul Jamo */
        (wc >= 0x2e80 && wc <= 0x9fff) ||   /* CJK */
        (wc >= 0xac00 && wc <= 0xd7a3) ||   /* Hangul Syllables */
        (wc >= 0xf900 && wc <= 0xfaff) ||   /* CJK Compat */
        (wc >= 0xfe10 && wc <= 0xfe1f) ||   /* Vertical Forms */
        (wc >= 0xfe30 && wc <= 0xfe6f) ||   /* CJK Compat Forms */
        (wc >= 0xff00 && wc <= 0xff60) ||   /* Fullwidth Forms */
        (wc >= 0xffe0 && wc <= 0xffe6))     /* Fullwidth Signs */
        return 2;
    return 1;
}

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
