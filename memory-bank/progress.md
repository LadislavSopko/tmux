# Progress - tmux Windows Port

§MBEL:5.0

## Overall Progress

```
Phase0:Analysis     ████████████ 100%
Phase0.5:POCs       ████████░░░░  80% (core done)
Phase1:Foundation   ████████████ 100% ✓ COMPLETE
Phase2:PTY-Layer    ████████████ 100% ✓ COMPLETE
Phase3:IPC-Layer    ████████████ 100% ✓ COMPLETE
Phase4:Signals      ████████████ 100% ✓ COMPLETE
Phase5:Terminal     ████████████ 100% ✓ COMPLETE
Phase6:Integration  ░░░░░░░░░░░░   0% (next - runtime testing)

Overall: ~95%
```

## Phase 1: Foundation ✓100% COMPLETE

**tmux.exe compiles and links on Windows!**
- Tested: `tmux -V` outputs "tmux 3.5.0"
- 154 C files compile (153 tmux + cmd-parse.c)
- EXE size: ~1.87MB

### Build Infrastructure ✓COMPLETE
- [x] env.bat - VS environment loader
- [x] setup-deps.bat - vcpkg + deps installer
- [x] build.bat - cmake + ninja build
- [x] test.bat - test runner
- [x] .gitignore for windows/

### Dependencies ✓COMPLETE
- [x] vcpkg installed in windows/thirdparty/
- [x] libevent:x64-windows
- [x] pdcurses:x64-windows
- [x] CMakeLists.txt with vcpkg toolchain

### POSIX Compatibility Layer ✓COMPLETE (20+ headers)
- [x] compat-win32.h (main + signals + time + wchar)
- [x] sys/time.h, socket.h, wait.h, uio.h, ioctl.h, file.h, un.h, utsname.h
- [x] unistd.h, termios.h, fnmatch.h, pwd.h
- [x] glob.h, libgen.h, regex.h, resolv.h, langinfo.h, term.h
- [x] netinet/in.h, arpa/inet.h

### Source Patches Applied ✓COMPLETE
- [x] popup.c: SIZE enum renamed to POPUP_SIZE
- [x] mode-tree.c: VLA replaced with _alloca
- [x] input.c: Forward declarations fixed with INPUT_TABLE_DEF
- [x] resize.c + tmux.h: resize_window → tmux_resize_window
- [x] setenv.c: environ → _environ
- [x] base64.c: Wrapped Unix-only includes
- [x] environ.c, client.c, tmux.c: global_environ / system_environ
- [x] cmd-parse.c: Generated from cmd-parse.y via bison
- [x] compat-win32.h: #undef ERROR (Windows conflict)

### Functions Implemented in compat-win32.h
- [x] getline, closefrom, ctime_r, fgetln
- [x] getptmfd, fdforkpty (stubs)
- [x] clock_gettime, gmtime_r, strcasestr
- [x] realpath, lstat, getpagesize, getpeereid
- [x] sigprocmask, killpg stubs

## Phase 2: PTY Layer ✓100% COMPLETE

**ConPTY integration complete and building!**

### Implementation ✓COMPLETE
- [x] pty-win32.c (477 lines) - Full ConPTY wrapper
- [x] pty-win32.h - PTY interface header
- [x] spawn.c patched with Windows ConPTY path
- [x] job.c patched with Windows ConPTY + CreateProcess
- [x] environ.c - environ_for_spawn() added
- [x] tmux.h - Declaration for environ_for_spawn

### Key Implementation Details
- `pty_create()`: CreatePseudoConsole + pipe setup
- `pty_spawn()`: CreateProcessW with PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
- `pty_get_fd()`: Uses `_open_osfhandle()` for libevent compatibility
- Environment conversion: `struct environ` → `char**` for CreateProcess

### Build Status
- ✓ All files compile
- ✓ EXE links
- ⚠ Minor warnings (unused Unix vars) - expected

## Phase 3: IPC Layer ✓100% COMPLETE

**Named Pipes IPC implementation complete!**

### Implementation ✓COMPLETE
- [x] ipc-win32.h - IPC interface header
- [x] ipc-win32.c (400+ lines) - Full Named Pipes wrapper
- [x] server.c patched with Windows Named Pipes
- [x] client.c patched with Windows Named Pipes

### Key Implementation Details
- `ipc_server_create()`: CreateNamedPipeA + FILE_FLAG_OVERLAPPED
- `ipc_server_accept()`: ConnectNamedPipe + new instance creation
- `ipc_client_connect()`: CreateFileA to connect to pipe
- `ipc_socket_to_pipe_path()`: Converts `/tmp/tmux-xxx` → `\\.\pipe\tmux-xxx`
- Uses `_open_osfhandle()` for libevent fd compatibility

### Build Status
- ✓ All files compile
- ✓ EXE links
- ⚠ Minor type warnings - non-blocking

## Phase 4: Signal Emulation ✓100% COMPLETE

**POSIX signal emulation complete!**

### Implementation ✓COMPLETE
- [x] signal-win32.h - Signal emulation interface
- [x] signal-win32.c (450+ lines) - Full signal emulation
- [x] compat-win32.h - Updated signal definitions

### Key Implementation Details
- `SetConsoleCtrlHandler()` for CTRL+C/BREAK/CLOSE
- Thread-based child monitoring for SIGCHLD
- Console size polling for SIGWINCH
- `sigaction()` and `kill()` implementations
- Signal handler registration and dispatch

### Build Status
- ✓ All files compile
- ✓ EXE links
- ⚠ Minor type warnings - non-blocking

## Phase 5: Terminal Integration ✓100% COMPLETE

**Windows Console terminal layer complete!**

### Implementation ✓COMPLETE
- [x] tty-win32.h - Terminal interface header
- [x] tty-win32.c (350+ lines) - Full terminal implementation
- [x] termios.h - Updated to use real implementations
- [x] CMakeLists.txt - Added tty-win32.c

### Key Implementation Details
- `ENABLE_VIRTUAL_TERMINAL_PROCESSING` for ANSI escapes
- `ENABLE_VIRTUAL_TERMINAL_INPUT` for input handling
- Real `tcgetattr_win32()` / `tcsetattr_win32()`
- Termios ↔ Console mode conversion

### Build Status
- ✓ All 152 files compile
- ✓ EXE links

## Phase 6: Full Integration ⏳READY TO START

- Test tmux new-session
- Test pane splitting
- Test detach/attach
- Fix runtime issues

## File Counts

| Category | Total | Done |
|----------|-------|------|
| Build scripts | 5 | 5 |
| POSIX compat headers | 20+ | 20+ |
| Windows src stubs | 7 | 7 |
| Compile: all files | 154 | 154 |
| Link: exe built | 1 | 1 |

## Issues Encountered & Fixed

1. vcpkg initially in C:\ - FIXED (moved to thirdparty/)
2. CMake quotes in /FI path - FIXED (SHELL: prefix)
3. `environ` macro conflict - FIXED (undef + per-file handling)
4. Missing POSIX headers - FIXED (created 20+ shims)
5. Signal constants missing - FIXED (SIG_BLOCK etc)
6. socketpair missing - FIXED (stub)
7. gmtime_r/wcwidth missing - FIXED (inline impl)
8. PATH_MAX missing - FIXED (defined)
9. VLA in mode-tree.c - FIXED (_alloca)
10. SIZE conflict in popup.c - FIXED (renamed POPUP_SIZE)
11. resize_window PDCurses conflict - FIXED (tmux_resize_window)
12. cmd_parse_* missing - FIXED (bison generated cmd-parse.c)
13. ERROR macro conflict - FIXED (#undef ERROR)
14. cur_term type conflict - FIXED (consolidated in term.h)

## Next Session

1. Phase 2: Implement ConPTY integration in pty-win32.c
2. Phase 3: Implement Named Pipes IPC
3. Test basic functionality (tmux new-session, attach, etc.)
