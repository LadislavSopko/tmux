# Active Context - tmux Windows Port

§MBEL:5.0

## Current Focus

@task::Phase2-PTY-Layer-Complete
⚡active::Phase 2 complete, ready for Phase 3 IPC

## Session 2026-01-29 - Phase 2 COMPLETE ✓

### Completed Work
1. **pty-win32.c** (477 lines) - Full ConPTY implementation:
   - `pty_create()` - Creates ConPTY with specified dimensions
   - `pty_spawn()` - Spawns process attached to ConPTY
   - `pty_read()` - Non-blocking read from PTY
   - `pty_write()` - Write to PTY
   - `pty_resize()` - Resize ConPTY
   - `pty_destroy()` - Cleanup all handles
   - `pty_get_fd()` - Returns fd for libevent integration
   - `fdforkpty()` - POSIX compat stub (returns -1, uses native path)

2. **spawn.c** - Windows ConPTY integration:
   - `#include "pty-win32.h"` for Windows
   - Windows-specific block at line 386+
   - Uses `pty_create(cols, rows)` + `pty_spawn(cmdline, envp)`
   - Uses `environ_for_spawn()` for environment conversion
   - Sets `new_wp->tty = "ConPTY"`
   - Skips fork child path (no fork on Windows)

3. **job.c** - Windows ConPTY + CreateProcess:
   - `#include "pty-win32.h"` for Windows
   - JOB_PTY: Uses ConPTY same as spawn.c
   - Non-PTY: Uses CreateProcess with anonymous pipes
   - Minor warnings for unused Unix-only variables

4. **environ.c** - `environ_for_spawn()` function:
   - Converts `struct environ` to `char**` array for CreateProcess

5. **tmux.h** - Declaration added:
   - `char **environ_for_spawn(struct environ *)` for Windows

### Build Status
- ✓ All 154 files compile
- ✓ tmux.exe links successfully
- ⚠ Minor warnings for unused Unix variables in job.c (expected)

### Next: Phase 3 - IPC Layer (Named Pipes)
- server.c: Replace AF_UNIX sockets with Named Pipes
- client.c: Replace connect() with Named Pipe client
- Reference: POC-02 (pocs/02-named-pipes/)
