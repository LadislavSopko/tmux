# Tech Context - tmux Windows Port

§MBEL:5.0

## Source Repository

@repo::github.com/tmux/tmux{fork}
@branch::master
@language::C{87%}

## Project Structure

```
tmux/                        # Root
├── *.c, *.h                 # Original tmux sources (~142 files)
├── compat/                  # POSIX compat implementations
├── pocs/                    # Windows API proof-of-concepts ✓VALIDATED
│   ├── 01-conpty/          # ConPTY test ✓WORKS
│   ├── 02-named-pipes/     # IPC test ✓WORKS
│   ├── 03-process/         # Process test ✓5/5
│   ├── 04-console-events/  # Signal test ~COMPILES
│   ├── 05-pdcurses/        # Curses test -SKIPPED
│   └── 06-libevent-win/    # Event loop -SKIPPED
├── windows/                 # Windows port (NEW)
│   ├── CMakeLists.txt      # Build system
│   ├── build.bat           # Build script
│   ├── docs/               # Documentation
│   ├── include/            # Windows headers
│   ├── src/                # Windows implementations
│   └── tests/              # Unit tests (TDD)
└── memory-bank/            # Project state
```

## Dependencies

[Required]
libevent::EventLoop{✓Windows-supported}
PDCurses::TerminalUI{ncurses-replacement}

[Windows SDK]
Windows10+{build-1809+}
ConPTY-API::Required
MSVC2019+|MinGW-w64::Compiler

[Link Libraries]
kernel32.lib::Process,file,console
advapi32.lib::Security,registry
user32.lib::Window-messages
ws2_32.lib::Winsock{optional}

## POSIX→Windows Mapping

| POSIX | Windows | Validated |
|-------|---------|-----------|
| forkpty() | CreatePseudoConsole() | ✓POC-01 |
| socket(AF_UNIX) | CreateNamedPipe() | ✓POC-02 |
| fork+exec | CreateProcess() | ✓POC-03 |
| waitpid | WaitForSingleObject() | ✓POC-03 |
| kill | TerminateProcess() | ✓POC-03 |
| signal/sigaction | SetConsoleCtrlHandler() | ~POC-04 |
| termios | Console API | ?TODO |

## Build Commands

[Windows Port - PRIMARY]
```batch
cd D:\Projekty\AI_Works\tmux\windows
build.bat
:: Output: windows/build/bin/*.exe
:: Runs: cmake configure + build + ctest
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

## Test Commands

[Run All Tests]
```batch
cd windows
build.bat
:: Automatically runs ctest
```

[Run Single Test]
```batch
cd windows/build/bin
test_pty.exe
test_ipc.exe
test_proc.exe
test_signal.exe
```

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
