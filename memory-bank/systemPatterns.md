# System Patterns - tmux Windows Port

§MBEL:5.0

## Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                    tmux core                         │
│         (business logic - DO NOT MODIFY)             │
│         ~90 files: cmd-*.c, format.c, grid.c        │
└──────────────────────┬──────────────────────────────┘
                       │
        ┌──────────────┼──────────────────┐
        ▼              ▼                  ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│ windows/src/ │ │ windows/src/ │ │ windows/src/ │
│ pty-win32.c  │ │ ipc-win32.c  │ │ proc-win32.c │
└──────────────┘ └──────────────┘ └──────────────┘
```

## Current Status

**Phase 1: COMPLETE ✓**
- tmux.exe compiles and links (1.87MB)
- 154/154 C files compile
- Runtime: `tmux -V` outputs "tmux 3.5.0"

**Next: Phase 2-6 Implementation**

## BBC Pattern (Black Box Composition)

[Principle]
@wrap::POSIXAPIs→AbstractInterface
@implement::Platform-specific{#ifdef _WIN32}
→Core¬KnowsAboutPlatform

## Development Methodology

[TDD - Test Driven Development]
@principle::TestFirst→ImplementAfter
@reference::POCs-are-acceptance-tests
→RED::WriteFailingTest
→GREEN::ImplementToPass
→REFACTOR::CleanUp

## Windows Port Structure

```
windows/
├── CMakeLists.txt           # Build system ✓
├── build.bat                # Build + test script ✓
├── docs/
│   ├── PORTING-PLAN.md      # Master plan with checkboxes
│   ├── TDD-STRATEGY.md      # Test strategy
│   └── OPERATIONAL-RULES.md # Workflow rules
├── include/
│   ├── compat-win32.h       # Main Windows compat ✓
│   └── [20+ POSIX headers]  # sys/, termios.h, etc. ✓
├── src/
│   ├── osdep-win32.c        # OS-dependent ✓
│   ├── pty-win32.c          # ConPTY wrapper (stub, TODO Phase 2)
│   ├── ipc-win32.c          # Named Pipes (stub, TODO Phase 3)
│   ├── proc-win32.c         # Process management (stub, TODO Phase 4)
│   ├── signal-win32.c       # Signal emulation (stub, TODO Phase 5)
│   ├── daemon-win32.c       # Background process (stub) ✓
│   └── imsg-win32.c         # Message protocol (stub) ✓
└── tests/
    └── [TODO: unit tests]
```

## Layer 1: PTY Abstraction

[Interface::windows/src/pty-win32.h]
```c
pty_handle_t* pty_create(cols, rows)
pid_t pty_spawn(pty, cmd, argv, env)
int pty_resize(pty, cols, rows)
ssize_t pty_read(pty, buf, len)
ssize_t pty_write(pty, buf, len)
void pty_destroy(pty)
```

[POSIX→Windows Mapping]
forkpty()→CreatePseudoConsole()
/dev/ptmx→HPCON handle
fork+dup2→CreateProcess+STARTUPINFOEX

[POC Reference]
pocs/01-conpty/→VALIDATED✓

## Layer 2: IPC Abstraction

[Interface::windows/src/ipc-win32.h]
```c
ipc_handle_t* ipc_listen(path)
ipc_handle_t* ipc_connect(path)
ipc_handle_t* ipc_accept(server)
ssize_t ipc_send(h, buf, len)
ssize_t ipc_recv(h, buf, len)
void ipc_close(h)
```

[POSIX→Windows Mapping]
socket(AF_UNIX)→CreateNamedPipe()
bind+listen→implicit in CreateNamedPipe
accept→ConnectNamedPipe()
connect→CreateFile(pipe)

[POC Reference]
pocs/02-named-pipes/→VALIDATED✓

## Layer 3: Process Abstraction

[Interface::windows/src/proc-win32.h]
```c
proc_handle_t* proc_spawn(cmd, argv, env, cwd)
int proc_wait(h, status, flags)
int proc_kill(h, signal)
void proc_close(h)
```

[POSIX→Windows Mapping]
fork+exec→CreateProcess()
waitpid→WaitForSingleObject()
kill→TerminateProcess()

[POC Reference]
pocs/03-process/→VALIDATED✓{5/5-tests}

## Layer 4: Signal Abstraction

[Interface::windows/src/signal-win32.h]
```c
sighandler_t win32_signal(sig, handler)
int win32_kill(pid, sig)
void win32_signal_init()
```

[POSIX→Windows Mapping]
signal/sigaction→SetConsoleCtrlHandler()
SIGCHLD→WaitForSingleObject()+callback
SIGINT/SIGTERM→CTRL_C_EVENT/CTRL_BREAK_EVENT
SIGWINCH→GetConsoleScreenBufferInfo() polling

[POC Reference]
pocs/04-console-events/→COMPILES~

## Key tmux Files to Wrap

[Critical - Heavy POSIX]
spawn.c:382→fdforkpty()→pty-win32
job.c:120→fork()→proc-win32
server.c:106→socket(AF_UNIX)→ipc-win32
client.c→connect()→ipc-win32
proc.c→sigaction()→signal-win32
tty.c→termios→Console API

[Partial Changes]
cmd-if-shell.c, cmd-run-shell.c, cmd-pipe-pane.c
server-fn.c, server-client.c

[Core - No Changes]
cmd-*.c (60+ files), format.c, grid.c, layout.c
screen.c, window.c, options.c, etc.

## Parallelization

[4 Independent Layers]
All depend only on compat-win32.h (DONE)
Can develop in parallel with TDD:
→Agent-PTY: pty-win32.c + test_pty.c
→Agent-IPC: ipc-win32.c + test_ipc.c
→Agent-PROC: proc-win32.c + test_proc.c
→Agent-SIGNAL: signal-win32.c + test_signal.c
