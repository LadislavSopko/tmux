# Active Context - tmux Windows Port

§MBEL:5.0

## Current Focus

@task::Phase1-Foundation
⚡active::POSIX-compat-95%-complete

## What Was Done This Session

[Session 2026-01-29 - Continued]

✓ Fixed multiple compilation errors iteratively:
  - Added SIG_BLOCK, SIG_SETMASK, SIG_UNBLOCK
  - Added socketpair stub
  - Created glob.h
  - Created libgen.h (basename/dirname)
  - Created netinet/in.h
  - Created regex.h (stub)
  - Created resolv.h (b64_ntop/b64_pton)
  - Created sys/utsname.h (uname)
  - Created langinfo.h (nl_langinfo)
  - Created arpa/inet.h
  - Created term.h (curses terminal defs)
  - Added SHUT_RD/WR/RDWR, killpg stub
  - Added gmtime_r, localtime_r
  - Added wcwidth
  - Added fseeko, ftello, mkstemp
  - Added PATH_MAX, NAME_MAX, MAXPATHLEN

✓ Build progress: 145/153 files compile (95%)

## Remaining Issues (8 files)

Files requiring source patches:
1. popup.c - SIZE enum conflicts with Windows typedef
2. mode-tree.c - VLA (Variable Length Array)
3. input.c - Designated initializer arrays with unknown size
4. tty.c - resize_window() conflicts with PDCurses
5. tty-term.c - term.h/curses integration
6. tty-keys.c - curses integration
7. setenv.c - environ undeclared
8. base64.c - minor include issue

## What To Do Next

1. Create minimal patches for VLA issues (mode-tree.c, popup.c)
2. Handle PDCurses symbol conflicts (resize_window)
3. Provide Windows-native setenv() replacement
4. Complete Phase 1 with 100% compilation
5. Then start Phase 2-6 TDD layers

## Blockers

8 files need source modifications - acceptable per BBC strategy for:
- C99 features not supported by MSVC (VLA)
- Windows naming conflicts (SIZE, resize_window)

## POSIX Headers Created (20 total)

```
windows/include/
├── compat-win32.h (main compat)
├── unistd.h
├── termios.h
├── fnmatch.h
├── pwd.h
├── glob.h
├── libgen.h
├── regex.h
├── resolv.h
├── langinfo.h
├── term.h
├── sys/
│   ├── time.h
│   ├── socket.h
│   ├── wait.h
│   ├── uio.h
│   ├── ioctl.h
│   ├── file.h
│   ├── un.h
│   └── utsname.h
├── netinet/
│   └── in.h
└── arpa/
    └── inet.h
```
