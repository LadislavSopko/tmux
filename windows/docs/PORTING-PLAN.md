# tmux Windows Port - Complete File-by-File Plan

## Overview

**Strategy:** BBC (Black Box Composition) - wrap POSIX APIs, don't modify core tmux code.

**Methodology:** TDD - Test-Driven Development. I POC esistenti sono i nostri test di riferimento.

**Progress Tracking:** Mark `[x]` when completed.

**Vedi anche:**
- `TDD-STRATEGY.md` - Strategia di testing
- `OPERATIONAL-RULES.md` - Regole operative, build commands, commit workflow

## Build & Test Commands

```batch
:: Build tutto
cd D:\Projekty\AI_Works\tmux\windows
build.bat

:: Output
windows/build/bin/*.exe
windows/build/bin/test_*.exe
```

## Regole per Ogni Task

```
PRIMA:  Aggiorna activeContext.md con @task::NomeTask
DURANTE: Aggiorna progress.md se problemi
DOPO:   [x] checkbox in questo file + COMMIT
```

---

## Part 1: Files to CREATE (Windows-Specific)

### 1.1 Core Windows Abstraction Layer (`windows/src/`)

| Done | File | Purpose | Replaces | Windows APIs |
|:----:|------|---------|----------|--------------|
| [~] | `pty-win32.c` | ConPTY wrapper | forkpty(), fdforkpty() | CreatePseudoConsole, CreateProcess |
| [ ] | `pty-win32.h` | PTY interface header | - | - |
| [~] | `ipc-win32.c` | Named Pipes IPC | Unix sockets (AF_UNIX) | CreateNamedPipe, ConnectNamedPipe |
| [ ] | `ipc-win32.h` | IPC interface header | - | - |
| [~] | `proc-win32.c` | Process management | fork(), exec(), waitpid() | CreateProcess, WaitForSingleObject |
| [ ] | `proc-win32.h` | Process interface header | - | - |
| [~] | `signal-win32.c` | Signal emulation | POSIX signals | SetConsoleCtrlHandler, Events |
| [ ] | `signal-win32.h` | Signal interface header | - | - |
| [x] | `osdep-win32.c` | OS-dependent functions | osdep-*.c | GetModuleFileName, etc. |
| [~] | `daemon-win32.c` | Background process | daemon() | CreateProcess (detached) |
| [~] | `imsg-win32.c` | Message protocol | imsg.c, imsg-buffer.c | Named Pipes + protocol |

**Note:** [~] = stub file created and compiles, implementation TODO in Phase 2-5

### 1.2 Compatibility Shims (`windows/include/`)

| Done | File | Purpose |
|:----:|------|---------|
| [x] | `compat-win32.h` | Main compat: signals, time, wchar, path macros |
| [x] | `unistd.h` | POSIX unistd.h replacements |
| [x] | `sys/socket.h` | Socket type definitions |
| [x] | `sys/time.h` | gettimeofday, timeval macros |
| [x] | `sys/wait.h` | wait macros |
| [x] | `sys/uio.h` | iovec, readv/writev |
| [x] | `sys/ioctl.h` | TIOCGWINSZ, winsize |
| [x] | `sys/file.h` | flock |
| [x] | `sys/un.h` | sockaddr_un |
| [x] | `sys/utsname.h` | uname structure |
| [x] | `termios.h` | Terminal I/O structures |
| [x] | `fnmatch.h` | Pattern matching |
| [x] | `pwd.h` | passwd struct, getpwuid |
| [x] | `glob.h` | glob pattern matching |
| [x] | `libgen.h` | basename, dirname |
| [x] | `regex.h` | POSIX regex (stub) |
| [x] | `resolv.h` | b64_ntop, b64_pton |
| [x] | `langinfo.h` | nl_langinfo |
| [x] | `term.h` | curses terminal definitions |
| [x] | `netinet/in.h` | Network byte order |
| [x] | `arpa/inet.h` | IP address conversion |

**Total: 20 compatibility headers created**

### 1.3 Build System (`windows/`)

| Done | File | Purpose |
|:----:|------|---------|
| [x] | `CMakeLists.txt` | Main CMake build file |
| [x] | `env.bat` | VS environment loader |
| [x] | `setup-deps.bat` | vcpkg + deps installer |
| [x] | `build.bat` | cmake + ninja build |
| [x] | `test.bat` | ctest runner |
| [x] | `.gitignore` | Ignore build/ and thirdparty/ |

**Note:** FindPDCurses.cmake and FindLibevent.cmake NOT needed - vcpkg toolchain handles discovery.

---

## Part 2: tmux Source Files - Porting Checklist

### 2.1 CRITICAL - Must Port/Wrap (12 files)

| Done | File | Lines | POSIX APIs | Action |
|:----:|------|-------|------------|--------|
| [ ] | `spawn.c` | 509 | fdforkpty, fork, exec, tcgetattr | WRAP with pty-win32 |
| [ ] | `job.c` | 450 | fork, forkpty, socketpair, waitpid | WRAP with proc-win32 |
| [ ] | `server.c` | 559 | socket(AF_UNIX), bind, listen, accept | WRAP with ipc-win32 |
| [ ] | `client.c` | 809 | socket(AF_UNIX), connect, flock | WRAP with ipc-win32 |
| [ ] | `proc.c` | 386 | sigaction, sigprocmask, fork | WRAP with signal-win32 |
| [ ] | `server-client.c` | 4034 | imsg, signals, ioctl | WRAP with imsg-win32 |
| [ ] | `tty.c` | 3030 | ioctl, termios, tcgetattr | WRAP with termios-win32 |
| [ ] | `control.c` | 1117 | imsg, read/write | WRAP with imsg-win32 |
| [ ] | `control-notify.c` | 262 | imsg | WRAP with imsg-win32 |
| [ ] | `file.c` | 868 | imsg | PARTIAL wrap |
| [ ] | `tmux.c` | 540 | getpwuid, getuid, signal | WRAP with compat-win32 |
| [ ] | `server-acl.c` | 186 | getuid, getgid | WRAP with compat-win32 |

### 2.2 PARTIAL CHANGES - Minor Modifications (8 files)

| Done | File | Lines | Issue | Action |
|:----:|------|-------|-------|--------|
| [ ] | `cmd-if-shell.c` | 190 | fork, execl | Use proc-win32 |
| [ ] | `cmd-pipe-pane.c` | 230 | fork, socket | Use proc-win32, ipc-win32 |
| [ ] | `cmd-run-shell.c` | 293 | shell execution | Use proc-win32 |
| [ ] | `cmd-new-session.c` | 370 | fork, daemon | Use wrappers |
| [ ] | `cmd-kill-server.c` | 61 | signal | Use signal-win32 |
| [ ] | `cmd-kill-session.c` | 71 | signal | Use signal-win32 |
| [ ] | `cmd-wait-for.c` | 264 | signal, waitpid | Use wrappers |
| [ ] | `server-fn.c` | 510 | signal | Use signal-win32 |

### 2.3 OSDEP - Create Windows Version

| Done | File | Status |
|:----:|------|--------|
| [x] | `osdep-win32.c` | CREATED (stub) |
| [ ] | `osdep_get_name()` | Implement fully |
| [ ] | `osdep_get_cwd()` | Implement fully |
| [ ] | `osdep_event_init()` | Implement fully |

### 2.4 CORE - Include As-Is (verify compilation)

**Commands (60+ files) - verify each compiles:**

| Done | Files |
|:----:|-------|
| [ ] | cmd-attach-session.c, cmd-bind-key.c, cmd-break-pane.c |
| [ ] | cmd-capture-pane.c, cmd-choose-tree.c, cmd-command-prompt.c |
| [ ] | cmd-confirm-before.c, cmd-copy-mode.c, cmd-detach-client.c |
| [ ] | cmd-display-menu.c, cmd-display-message.c, cmd-display-panes.c |
| [ ] | cmd-find-window.c, cmd-find.c, cmd-join-pane.c |
| [ ] | cmd-kill-pane.c, cmd-kill-window.c, cmd-list-buffers.c |
| [ ] | cmd-list-clients.c, cmd-list-keys.c, cmd-list-panes.c |
| [ ] | cmd-list-sessions.c, cmd-list-windows.c, cmd-load-buffer.c |
| [ ] | cmd-lock-server.c, cmd-move-window.c, cmd-new-window.c |
| [ ] | cmd-paste-buffer.c, cmd-queue.c, cmd-refresh-client.c |
| [ ] | cmd-rename-session.c, cmd-rename-window.c, cmd-resize-pane.c |
| [ ] | cmd-resize-window.c, cmd-respawn-pane.c, cmd-respawn-window.c |
| [ ] | cmd-rotate-window.c, cmd-save-buffer.c, cmd-select-layout.c |
| [ ] | cmd-select-pane.c, cmd-select-window.c, cmd-send-keys.c |
| [ ] | cmd-server-access.c, cmd-set-buffer.c, cmd-set-environment.c |
| [ ] | cmd-set-option.c, cmd-show-environment.c, cmd-show-messages.c |
| [ ] | cmd-show-options.c, cmd-show-prompt-history.c, cmd-source-file.c |
| [ ] | cmd-split-window.c, cmd-swap-pane.c, cmd-swap-window.c |
| [ ] | cmd-switch-client.c, cmd-unbind-key.c, cmd.c |

**Core Logic (30+ files):**

| Done | Files |
|:----:|-------|
| [ ] | arguments.c, alerts.c, attributes.c, cfg.c, colour.c |
| [ ] | environ.c, format.c, format-draw.c |
| [ ] | grid.c, grid-view.c, grid-reader.c |
| [ ] | hyperlinks.c, image.c, image-sixel.c |
| [ ] | input.c, input-keys.c |
| [ ] | key-bindings.c, key-string.c |
| [ ] | layout.c, layout-set.c, layout-custom.c |
| [ ] | log.c, menu.c, mode-tree.c, names.c, notify.c |
| [ ] | options.c, options-table.c |
| [ ] | paste.c, popup.c, regsub.c, resize.c |
| [ ] | screen.c, screen-write.c, screen-redraw.c |
| [ ] | session.c, status.c, style.c |
| [ ] | tty-acs.c, tty-draw.c, tty-features.c, tty-keys.c, tty-term.c |
| [ ] | utf8.c, utf8-combined.c |
| [ ] | window.c, window-buffer.c, window-client.c, window-clock.c |
| [ ] | window-copy.c, window-customize.c, window-tree.c |
| [ ] | xmalloc.c |

**Headers:**

| Done | File |
|:----:|------|
| [ ] | tmux.h - verify no POSIX-only includes |
| [ ] | tmux-protocol.h |
| [ ] | compat.h - extend for Windows |
| [ ] | xmalloc.h |

**Parser:**

| Done | File |
|:----:|------|
| [ ] | cmd-parse.y - verify yacc works on Windows |

---

## Part 3: Compat Files Checklist

### 3.1 REPLACE - Need Windows Implementation (15 files)

| Done | File | Purpose | Windows Replacement |
|:----:|------|---------|---------------------|
| [ ] | `fdforkpty.c` | PTY wrapper | pty-win32.c |
| [ ] | `forkpty-*.c` (5 files) | Platform PTY | pty-win32.c |
| [ ] | `imsg.c` | IPC protocol | imsg-win32.c |
| [ ] | `imsg-buffer.c` | IPC buffers | imsg-win32.c |
| [ ] | `daemon.c` | Daemonize | daemon-win32.c |
| [ ] | `closefrom.c` | Close FDs | CloseHandle loop |
| [ ] | `getdtablesize.c` | FD limit | GetProcessHandleCount |
| [ ] | `getdtablecount.c` | FD count | GetProcessHandleCount |
| [ ] | `getpeereid.c` | Socket peer ID | GetNamedPipeClientProcessId |
| [ ] | `cfmakeraw.c` | Raw terminal | SetConsoleMode |
| [ ] | `clock_gettime.c` | High-res time | QueryPerformanceCounter |

### 3.2 USE AS-IS - Pure C Utilities (32 files)

| Done | Category | Files |
|:----:|----------|-------|
| [ ] | String | strlcpy.c, strlcat.c, strndup.c, strnlen.c, strsep.c, strcasestr.c, strtonum.c, memmem.c, asprintf.c |
| [ ] | Error/IO | err.c, getline.c, fgetln.c, getopt_long.c, getprogname.c |
| [ ] | Encoding | base64.c, htonll.c, ntohll.c, vis.c, unvis.c, utf8proc.c |
| [ ] | Memory | explicit_bzero.c, freezero.c, reallocarray.c, recallocarray.c |
| [ ] | Misc | setenv.c, setproctitle.c |
| [ ] | Headers | queue.h, tree.h, bitstring.h, vis.h |

### 3.3 SKIP - Not Needed (2 files)

| Done | File | Reason |
|:----:|------|--------|
| [x] | `systemd.c` | Linux only - SKIP |
| [x] | `daemon-darwin.c` | macOS only - SKIP |

---

## Part 4: Dependencies Checklist

### 4.1 External Libraries

| Done | Library | Purpose | Source |
|:----:|---------|---------|--------|
| [x] | libevent | Event loop | vcpkg install libevent:x64-windows |
| [x] | PDCurses | Terminal UI | vcpkg install pdcurses:x64-windows |

**Note:** Installed in `windows/thirdparty/vcpkg/` via `setup-deps.bat`

### 4.2 Windows SDK

| Done | Requirement |
|:----:|-------------|
| [x] | Windows 10 SDK (10.0.17763.0+) installed |
| [x] | MSVC 2019+ installed (via VS environment) |
| [x] | ConPTY API available (Win10 1809+) |

### 4.3 System Libraries (link)

| Done | Library | Purpose |
|:----:|---------|---------|
| [x] | kernel32.lib | Process, file, console |
| [x] | advapi32.lib | Security, registry |
| [x] | user32.lib | Window messages |
| [x] | ws2_32.lib | Winsock (for libevent) |

**Note:** All linked in CMakeLists.txt `target_link_libraries()`

---

## Part 5: Implementation Phases

### Phase 1: Foundation

| Done | Task | Description |
|:----:|------|-------------|
| [x] | Create windows/ folder | mkdir windows/{src,include,docs} |
| [x] | Create CMakeLists.txt | Build with vcpkg toolchain |
| [x] | Create compat-win32.h | Defines, macros, signal stubs |
| [x] | Create osdep-win32.c | Stub with 3 functions |
| [x] | Create POSIX compat headers | 20 shim headers in include/ |
| [x] | Create build infrastructure | env.bat, setup-deps.bat, build.bat, test.bat |
| [x] | Install vcpkg | In windows/thirdparty/vcpkg |
| [x] | Install libevent | vcpkg install libevent:x64-windows |
| [x] | Install PDCurses | vcpkg install pdcurses:x64-windows |
| [x] | First compile attempt | 145/153 files compile (95%) |
| [ ] | All tmux files compile | 8 files need source patches |

**Phase 1 Status: 95% complete. 8 files require minimal source patches for:**
- VLA (Variable Length Arrays) - MSVC doesn't support C99 VLAs
- Naming conflicts (SIZE, resize_window vs Windows/PDCurses)

### Phase 2: PTY Layer (TDD)

**POC Reference:** `pocs/01-conpty/` (WORKING), `pocs/03-process/` (5/5 PASSED)

| Done | Task | Description | TDD |
|:----:|------|-------------|:---:|
| [ ] | Create tests/test_pty.c | Test file FIRST | RED |
| [ ] | Write test_pty_create() | Test CreatePseudoConsole | RED |
| [ ] | Create pty-win32.h | Define PTY interface | - |
| [ ] | Create pty-win32.c (stub) | Empty implementation | RED |
| [ ] | Implement pty_create() | Make test pass | GREEN |
| [ ] | Write test_pty_spawn() | Test spawn cmd.exe | RED |
| [ ] | Implement pty_spawn() | Make test pass | GREEN |
| [ ] | Write test_pty_resize() | Test resize | RED |
| [ ] | Implement pty_resize() | Make test pass | GREEN |
| [ ] | Write test_pty_read_write() | Test I/O | RED |
| [ ] | Implement pty_read/write() | Make test pass | GREEN |
| [ ] | Create tests/test_proc.c | Test file FIRST | RED |
| [ ] | Write test_proc_spawn() | Test CreateProcess | RED |
| [ ] | Create proc-win32.h | Define process interface | - |
| [ ] | Create proc-win32.c (stub) | Empty implementation | RED |
| [ ] | Implement proc_spawn() | Make test pass (copy POC-03) | GREEN |
| [ ] | Write test_proc_wait() | Test wait | RED |
| [ ] | Implement proc_wait() | Make test pass | GREEN |
| [ ] | Write test_proc_kill() | Test terminate | RED |
| [ ] | Implement proc_kill() | Make test pass | GREEN |
| [ ] | All tests PASS | Verify all green | ✓ |
| [ ] | Adapt spawn.c | #ifdef _WIN32 blocks | - |
| [ ] | Adapt job.c | #ifdef _WIN32 blocks | - |

### Phase 3: IPC Layer (TDD)

**POC Reference:** `pocs/02-named-pipes/` (WORKING - bidirectional OK)

| Done | Task | Description | TDD |
|:----:|------|-------------|:---:|
| [ ] | Create tests/test_ipc.c | Test file FIRST | RED |
| [ ] | Write test_ipc_listen() | Test CreateNamedPipe | RED |
| [ ] | Create ipc-win32.h | Define IPC interface | - |
| [ ] | Create ipc-win32.c (stub) | Empty implementation | RED |
| [ ] | Implement ipc_listen() | Make test pass | GREEN |
| [ ] | Write test_ipc_connect() | Test connect to pipe | RED |
| [ ] | Implement ipc_connect() | Make test pass | GREEN |
| [ ] | Write test_ipc_accept() | Test accept connection | RED |
| [ ] | Implement ipc_accept() | Make test pass | GREEN |
| [ ] | Write test_ipc_send_recv() | Test bidirectional | RED |
| [ ] | Implement ipc_send/recv() | Make test pass | GREEN |
| [ ] | All IPC tests PASS | Verify all green | ✓ |
| [ ] | Create tests/test_imsg.c | Test message protocol | RED |
| [ ] | Write test_imsgbuf_init() | Test buffer init | RED |
| [ ] | Create imsg-win32.c (stub) | Empty implementation | RED |
| [ ] | Implement imsgbuf_init() | Make test pass | GREEN |
| [ ] | Write test_imsg_send_recv() | Test message exchange | RED |
| [ ] | Implement imsg protocol | Make test pass | GREEN |
| [ ] | All imsg tests PASS | Verify all green | ✓ |
| [ ] | Adapt server.c | Use ipc-win32 | - |
| [ ] | Adapt client.c | Use ipc-win32 | - |

### Phase 4: Signal Layer (TDD)

**POC Reference:** `pocs/04-console-events/` (compiles, needs testing)

| Done | Task | Description | TDD |
|:----:|------|-------------|:---:|
| [ ] | Verify POC-04 works | Manual test first | - |
| [ ] | Create tests/test_signal.c | Test file FIRST | RED |
| [ ] | Write test_signal_handler() | Test handler registration | RED |
| [ ] | Create signal-win32.h | Define signal interface | - |
| [ ] | Create signal-win32.c (stub) | Empty implementation | RED |
| [ ] | Implement win32_signal() | Make test pass | GREEN |
| [ ] | Write test_signal_ctrl_c() | Test CTRL_C_EVENT | RED |
| [ ] | Implement SIGINT handler | Make test pass | GREEN |
| [ ] | Write test_signal_child() | Test child exit detection | RED |
| [ ] | Implement SIGCHLD emulation | Make test pass | GREEN |
| [ ] | Write test_signal_kill() | Test kill() equivalent | RED |
| [ ] | Implement win32_kill() | Make test pass | GREEN |
| [ ] | All signal tests PASS | Verify all green | ✓ |
| [ ] | Adapt proc.c | Use signal-win32 | - |
| [ ] | Integration: Ctrl+C | Graceful handling | - |
| [ ] | Integration: child exit | Job cleanup works | - |

### Phase 5: Terminal Layer (TDD)

**POC Reference:** `pocs/05-pdcurses/` (needs dependency install)

| Done | Task | Description | TDD |
|:----:|------|-------------|:---:|
| [ ] | Create tests/test_termios.c | Test file FIRST | RED |
| [ ] | Write test_cfmakeraw() | Test raw mode | RED |
| [ ] | Create termios-win32.h | Terminal structures | - |
| [ ] | Implement cfmakeraw() | Make test pass | GREEN |
| [ ] | Write test_tcgetattr() | Test get console mode | RED |
| [ ] | Implement tcgetattr() | Make test pass | GREEN |
| [ ] | Write test_tcsetattr() | Test set console mode | RED |
| [ ] | Implement tcsetattr() | Make test pass | GREEN |
| [ ] | All termios tests PASS | Verify all green | ✓ |
| [ ] | Adapt tty.c | Use Console API | - |
| [ ] | Integrate PDCurses | Link and test | - |
| [ ] | Integration: basic output | Text renders correctly | - |
| [ ] | Integration: colors | ANSI colors work | - |
| [ ] | Integration: resize | Window resize works | - |

### Phase 6: Integration

| Done | Task | Description |
|:----:|------|-------------|
| [ ] | Full build | All files compile |
| [ ] | Link succeeds | tmux.exe created |
| [ ] | Test: tmux new | Create session |
| [ ] | Test: tmux split | Split panes |
| [ ] | Test: tmux detach | Detach works |
| [ ] | Test: tmux attach | Reattach works |
| [ ] | Test: tmux kill | Kill session |
| [ ] | Fix bugs | Address issues |
| [ ] | Documentation | Update README |

---

## Part 6: Parallelization Strategy

**I 4 layer possono essere sviluppati in parallelo dopo Phase 1 completata.**

Ogni layer dipende SOLO da `compat-win32.h` (già fatto).

```
                    compat-win32.h (DONE)
                           │
       ┌───────────────────┼───────────────────┐
       │                   │                   │
       ▼                   ▼                   ▼
   ┌─────────┐       ┌─────────┐       ┌─────────┐       ┌─────────┐
   │ Agent 1 │       │ Agent 2 │       │ Agent 3 │       │ Agent 4 │
   │ PTY+PROC│       │ IPC     │       │ SIGNAL  │       │ TERMINAL│
   │         │       │         │       │         │       │         │
   │ POC-01  │       │ POC-02  │       │ POC-04  │       │ POC-05  │
   │ POC-03  │       │         │       │         │       │         │
   └────┬────┘       └────┬────┘       └────┬────┘       └────┬────┘
        │                 │                 │                 │
        ▼                 ▼                 ▼                 ▼
   test_pty.c        test_ipc.c      test_signal.c    test_termios.c
   test_proc.c       test_imsg.c
   pty-win32.c       ipc-win32.c     signal-win32.c   termios-win32.h
   proc-win32.c      imsg-win32.c
```

### Prerequisiti per Parallelizzazione

| Done | Prerequisito |
|:----:|-------------|
| [x] | compat-win32.h creato |
| [x] | POSIX shim headers creati (11 files) |
| [x] | Build infrastructure funzionante |
| [ ] | Test framework scelto (assert/Unity/CMocka) |
| [ ] | CMakeLists.txt supporta test target |
| [ ] | Ogni agente ha POC reference assegnato |

### Assegnazione Agenti

| Agente | POC Reference | Output Files | Test Files |
|--------|---------------|--------------|------------|
| Agent-PTY | POC-01, POC-03 | pty-win32.c, proc-win32.c | test_pty.c, test_proc.c |
| Agent-IPC | POC-02 | ipc-win32.c, imsg-win32.c | test_ipc.c, test_imsg.c |
| Agent-SIGNAL | POC-04 | signal-win32.c | test_signal.c |
| Agent-TERMINAL | POC-05 | termios-win32.h | test_termios.c |

---

## Part 7: Summary Counts

| Category | Total | Done | Remaining |
|----------|-------|------|-----------|
| Files to Create (windows/src/) | 14 | 7 | 7 |
| POSIX Compat Headers | 20 | 20 | 0 |
| Build Scripts | 5 | 5 | 0 |
| tmux Files Compiling | 153 | 145 | 8 |
| Test Files to Create | 6 | 0 | 6 |
| Critical Files to Port | 12 | 0 | 12 |
| Partial Changes | 8 | 0 | 8 |
| Compat Replace | 15 | 0 | 15 |
| Compat Use As-Is | 32 | 0 | 32 |
| Phase 1 Tasks | 11 | 10 | 1 |
| Phase 2 Tasks (TDD) | 23 | 0 | 23 |
| Phase 3 Tasks (TDD) | 21 | 0 | 21 |
| Phase 4 Tasks (TDD) | 16 | 0 | 16 |
| Phase 5 Tasks (TDD) | 14 | 0 | 14 |
| Phase 6 Tasks | 9 | 0 | 9 |

**Overall Progress: ~45% of Phase 1 infrastructure, ready for TDD phases after full compilation**

### Test Files Checklist

| Done | Test File | Tests Layer |
|:----:|-----------|-------------|
| [ ] | tests/test_pty.c | PTY (ConPTY) |
| [ ] | tests/test_proc.c | Process management |
| [ ] | tests/test_ipc.c | IPC (Named Pipes) |
| [ ] | tests/test_imsg.c | Message protocol |
| [ ] | tests/test_signal.c | Signal emulation |
| [ ] | tests/test_termios.c | Terminal I/O |

---

## Part 8: POSIX → Windows Quick Reference

| POSIX | Windows |
|-------|---------|
| fork() | CreateProcess() |
| exec*() | CreateProcess() |
| waitpid() | WaitForSingleObject() |
| kill() | TerminateProcess() |
| forkpty() | CreatePseudoConsole() |
| socket(AF_UNIX) | CreateNamedPipe() |
| connect() | CreateFile() (pipe) |
| accept() | ConnectNamedPipe() |
| signal() | SetConsoleCtrlHandler() |
| ioctl() | DeviceIoControl() |
| tcgetattr() | GetConsoleMode() |
| tcsetattr() | SetConsoleMode() |
| getuid() | GetUserName() |
| chdir() | SetCurrentDirectory() |
| getcwd() | GetCurrentDirectory() |

---

*Last Updated: 2026-01-29*
*Total Tasks: ~170*
*Completed: ~55 (Phase 1 at 95%, 145/153 files compile)*
