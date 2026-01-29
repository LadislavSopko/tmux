# Tech Context - tmux Windows Port

§MBEL:5.0

## Source Repository

@repo::github.com/tmux/tmux{fork}
@branch::master
@language::C{87%}

## Current Dependencies

[Unix]
libevent::EventLoop{async-io}→✓Windows-supported!
ncurses::TerminalUI→PDCurses{drop-in-replacement}
POSIX-PTY::PseudoTerminal→ConPTY
fork()::ProcessCreation→CreateProcess()
Unix-sockets::IPC→NamedPipes
POSIX-signals::Events→Windows-events

## Windows Target

[Requirements]
Windows10+{build-1809+}
ConPTY-API::Required
MSVC|MinGW::Compiler

[Win32-APIs]
```c
// PTY
#include <windows.h>
#include <consoleapi.h>
CreatePseudoConsole()
ResizePseudoConsole()
ClosePseudoConsole()

// IPC
CreateNamedPipe()
ConnectNamedPipe()
CreateFile(){pipe}

// Process
CreateProcess()
WaitForSingleObject()
TerminateProcess()

// Events
SetConsoleCtrlHandler()
CreateEvent()
```

## Build System

[Current::autotools]
configure.ac
Makefile.am
autogen.sh

[Target::CMake]
?CMakeLists.txt→CREATE
→Cross-platform
→MSVC+MinGW+GCC

## Key Build Defines

```c
#ifdef _WIN32
  // Windows-specific code
#else
  // POSIX code
#endif

#ifdef HAVE_FORKPTY
#ifdef HAVE_FDFORKPTY
#ifdef HAVE_SYSTEMD
```

## File Structure

[Root~100files]
tmux.c::main-entry
tmux.h::main-header{114KB}
compat.h::compat-declarations

[Commands~60files]
cmd-*.c::tmux-commands{¬modify}

[Core~30files]
server.c::server-main
client.c::client-main
spawn.c::process-spawn
job.c::background-jobs
proc.c::process-lifecycle

[Terminal~15files]
tty.c::terminal-output
tty-*.c::terminal-features
input.c::input-parsing

[compat/~50files]
forkpty-*.c::pty-implementations
daemon.c::daemonization
Various-POSIX-compat

[osdep/]
osdep-*.c::OS-specific{get_name,get_cwd,event_init}

## Dependencies to Add (Windows)

[Libraries]
pdcurses::ncurses-replacement
libevent::already-supports-windows

[Link]
kernel32.lib
user32.lib
advapi32.lib
ws2_32.lib{maybe}

## Development Commands

[Current-Unix]
```bash
./autogen.sh
./configure
make
make install
```

[Target-Windows]
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Test Commands

[BasicTest]
```bash
# Create session
tmux new-session -d -s test

# List
tmux list-sessions

# Panes
tmux split-window -h
tmux split-window -v

# Interact
tmux send-keys "echo hello" Enter

# Detach/Attach
# Ctrl+B, D
tmux attach -t test

# Kill
tmux kill-session -t test
```

## Resources

[ConPTY]
docs.microsoft.com/windows/console/creating-a-pseudoconsole-session
github.com/microsoft/terminal{reference}
github.com/microsoft/terminal/tree/main/samples/ConPTY

[NamedPipes]
docs.microsoft.com/windows/win32/ipc/named-pipe-server-using-overlapped-i-o

[PDCurses]
github.com/wmcbrine/PDCurses
github.com/Bill-Gray/PDCursesMod{more-active}
