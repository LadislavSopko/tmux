# Tech Context - tmux Windows Port

§MBEL:5.0

## Source Repository

@repo::github.com/tmux/tmux{fork}
@branch::master
@language::C{87%}

## Build Status

**Phase 1: COMPLETE ✓**
- tmux.exe: 1.87MB
- 154/154 C files compile
- `tmux -V` outputs "tmux 3.5.0"

## Project Structure

```
tmux/                        # Root
├── *.c, *.h                 # Original tmux sources (~142 files)
├── cmd-parse.c              # Generated from cmd-parse.y (tracked for Windows)
├── compat/                  # POSIX compat implementations
├── pocs/                    # Windows API proof-of-concepts ✓VALIDATED
│   ├── 01-conpty/          # ConPTY test ✓WORKS
│   ├── 02-named-pipes/     # IPC test ✓WORKS
│   ├── 03-process/         # Process test ✓5/5
│   ├── 04-console-events/  # Signal test ~COMPILES
│   ├── 05-pdcurses/        # Curses test -SKIPPED
│   └── 06-libevent-win/    # Event loop -SKIPPED
├── windows/                 # Windows port ✓PHASE1-COMPLETE
│   ├── CMakeLists.txt      # Build system ✓
│   ├── build.bat           # Build script ✓
│   ├── build/bin/tmux.exe  # OUTPUT ✓
│   ├── docs/               # Documentation
│   ├── include/            # 20+ Windows headers ✓
│   ├── src/                # Windows implementations (stubs)
│   └── tests/              # Unit tests (TDD)
└── memory-bank/            # Project state
```

## Dependencies

[Installed via vcpkg]
libevent::2.2.0{✓Windows-supported}
pdcurses::3.9{✓Windows-supported}
→vcpkg in windows/thirdparty/vcpkg

[Windows SDK]
Windows10+{build-1809+}
ConPTY-API::Required
MSVC2022+::Compiler

[Link Libraries]
kernel32.lib, advapi32.lib, user32.lib, ws2_32.lib
event_core.lib, pdcurses.lib

## POSIX→Windows Mapping

| POSIX | Windows | Status |
|-------|---------|--------|
| forkpty() | CreatePseudoConsole() | ✓POC-01, stub in pty-win32.c |
| socket(AF_UNIX) | CreateNamedPipe() | ✓POC-02, stub in ipc-win32.c |
| fork+exec | CreateProcess() | ✓POC-03, stub in proc-win32.c |
| waitpid | WaitForSingleObject() | ✓POC-03 |
| kill | TerminateProcess() | ✓POC-03 |
| signal/sigaction | SetConsoleCtrlHandler() | ~POC-04, stub in signal-win32.c |
| termios | Console API | stub |

## Build Commands

[Windows Port - PRIMARY]
```batch
cd D:\Projekty\AI_Works\tmux\windows
build.bat
:: Output: windows/build/bin/tmux.exe
:: Runs: cmake configure + build + ctest
```

[Test Version]
```batch
windows\build\bin\tmux.exe -V
:: Output: tmux 3.5.0
```

[POCs - Reference]
```batch
cd D:\Projekty\AI_Works\tmux\pocs
build.bat
:: Output: pocs/build/bin/*.exe
```

[Manual Build]
```batch
:: Load VS environment
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Configure + Build
cmake -B build -G Ninja
cmake --build build

:: Run tests
ctest --test-dir build --output-on-failure
```

## Source Patches Applied

[Files Patched for Windows]
- popup.c: SIZE enum → POPUP_SIZE
- mode-tree.c: VLA → _alloca
- input.c: Forward declarations with INPUT_TABLE_DEF
- resize.c + tmux.h: resize_window → tmux_resize_window
- environ.c, client.c, tmux.c: environ handling
- compat/base64.c, compat/setenv.c: include fixes

[compat-win32.h Additions]
- #undef ERROR (Windows macro conflict)
- #undef SIZE (Windows typedef conflict)
- closefrom, getline, ctime_r implementations
- getptmfd, fdforkpty stubs
- 20+ POSIX headers in windows/include/

## Development Workflow

[TDD Cycle]
1. Write test in tests/test_*.c (RED)
2. Implement in src/*-win32.c (GREEN)
3. Refactor if needed
4. Commit

[File Locations]
| Cosa | Path |
|------|------|
| Piano | windows/docs/PORTING-PLAN.md |
| Regole | windows/docs/OPERATIONAL-RULES.md |
| TDD | windows/docs/TDD-STRATEGY.md |
| Source | windows/src/*.c |
| Tests | windows/tests/*.c |
| Headers | windows/include/*.h |

## Key tmux Files Analysis

[Critical - 12 files, heavy POSIX]
spawn.c{509}→fdforkpty,fork,exec
job.c{450}→fork,forkpty,socketpair
server.c{559}→socket,bind,listen,accept
client.c{809}→socket,connect,flock
proc.c{386}→sigaction,sigprocmask
server-client.c{4034}→imsg,signals
tty.c{3030}→ioctl,termios
control.c{1117}→imsg
control-notify.c{262}→imsg
file.c{868}→imsg
tmux.c{540}→getpwuid,signal
server-acl.c{186}→getuid,getgid

[Core - 90+ files, no changes needed]
cmd-*.c, format.c, grid.c, layout.c, screen.c, etc.

## Resources

[ConPTY]
docs.microsoft.com/windows/console/creating-a-pseudoconsole-session
github.com/microsoft/terminal/tree/main/samples/ConPTY

[NamedPipes]
docs.microsoft.com/windows/win32/ipc/named-pipe-server-using-overlapped-i-o

[PDCurses]
github.com/Bill-Gray/PDCursesMod{recommended}
