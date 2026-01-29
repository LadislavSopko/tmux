# System Patterns - tmux Windows Port

§MBEL:5.0

## Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                    tmux core                         │
│         (business logic - DO NOT MODIFY)             │
└──────────────────────┬──────────────────────────────┘
                       │
        ┌──────────────┼──────────────────┐
        ▼              ▼                  ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│  compat/     │ │  compat/     │ │  compat/     │
│  pty-win32   │ │  ipc-win32   │ │  proc-win32  │
└──────────────┘ └──────────────┘ └──────────────┘
```

## BBC Pattern (Black Box Composition)

[Principle]
@wrap::POSIXAPIs→AbstractInterface
@implement::Platform-specific{#ifdef _WIN32}
→Core¬KnowsAboutPlatform

## Layer 1: PTY Abstraction

[Interface::compat/pty.h]
```c
pty_handle_t* pty_create(cols, rows)
pid_t pty_spawn(pty, cmd, argv, env)
int pty_resize(pty, cols, rows)
ssize_t pty_read(pty, buf, len)
ssize_t pty_write(pty, buf, len)
void pty_destroy(pty)
```

[POSIX::forkpty()]
spawn.c:382→fdforkpty()→forkpty()
/dev/ptmx→grantpt→unlockpt→ptsname→fork→dup2

[Windows::ConPTY]
CreatePseudoConsole()→HPCON
CreateProcess()→STARTUPINFOEX{hPC}
ReadFile/WriteFile{hInput,hOutput}
ResizePseudoConsole()
ClosePseudoConsole()

## Layer 2: IPC Abstraction

[Interface::compat/ipc.h]
```c
ipc_handle_t* ipc_listen(path)
ipc_handle_t* ipc_connect(path)
ipc_handle_t* ipc_accept(server)
ssize_t ipc_send(h, buf, len)
ssize_t ipc_recv(h, buf, len)
void ipc_close(h)
```

[POSIX::UnixSockets]
server.c:106→server_create_socket()
AF_UNIX+SOCK_STREAM
socket→bind→listen→accept

[Windows::NamedPipes]
\\.\pipe\tmux-{username}-{session}
CreateNamedPipe()→HANDLE
ConnectNamedPipe()
CreateFile(){OPEN_EXISTING}

## Layer 3: Process Abstraction

[Interface::compat/proc.h]
```c
proc_handle_t* proc_spawn(cmd, argv, env, cwd)
int proc_wait(h, status, flags)
int proc_kill(h, signal)
void proc_close(h)
```

[POSIX::fork/exec]
job.c:120→fork()
spawn.c:382→fdforkpty()
execl/execvp

[Windows::CreateProcess]
CreateProcess()→PROCESS_INFORMATION
WaitForSingleObject()
TerminateProcess()

## Layer 4: Signal Abstraction

[Interface::compat/signal-win32.h]
```c
void signal_init()
void signal_set_handler(sig, handler)
void signal_child_watch(callback)
```

[POSIX::Signals]
server.c:430→server_signal()
SIGCHLD→waitpid()
SIGTERM/SIGINT→shutdown
SIGWINCH→resize

[Windows::Events]
SetConsoleCtrlHandler()→CTRL_C_EVENT
WaitForSingleObject()→child exit
Manual→resize polling

## Key Files by Layer

[PTY]
spawn.c→usespty
job.c→usespty{JOB_PTY flag}
compat/fdforkpty.c→wrapper
compat/forkpty-*.c→platform-impl
?compat/pty-win32.c→CREATE

[IPC]
server.c→socket-server
client.c→socket-client
?compat/ipc-win32.c→CREATE

[Process]
job.c→fork+exec
proc.c→process-lifecycle
?compat/proc-win32.c→CREATE

[Signal]
server.c→signal-handlers
tmux.c→signal-setup
?compat/signal-win32.c→CREATE

## Existing Platform Support

[osdep-*.c]
osdep-linux.c
osdep-freebsd.c
osdep-openbsd.c
osdep-darwin.c
osdep-cygwin.c→EXISTS!{reference}
?osdep-win32.c→CREATE

[compat/forkpty-*.c]
forkpty-sunos.c→reference-impl
forkpty-aix.c
forkpty-haiku.c
forkpty-hpux.c
?forkpty-win32.c→CREATE
