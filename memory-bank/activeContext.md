# Active Context - tmux Windows Port

§MBEL:5.0

## Current Focus

@task::Phase1-Foundation
⚡active::CompleteRemainingSetupTasks

## What To Do Next

1. Install libevent (vcpkg)
2. Install PDCurses (vcpkg)
3. Create cmake/FindPDCurses.cmake
4. Create cmake/FindLibevent.cmake
5. First compile attempt
6. Fix errors until headers compile

## Completed

[Session 2024-01-29]
✓POCs validated (ConPTY, NamedPipes, Process)
✓windows/ folder structure created
✓PORTING-PLAN.md with TDD checkboxes
✓TDD-STRATEGY.md
✓OPERATIONAL-RULES.md
✓build.bat
✓compat-win32.h
✓osdep-win32.c (stub)
✓CLAUDE.md updated with session procedure

## POC Status (Reference)

| POC | Status |
|-----|--------|
| 01-conpty | ✓WORKS |
| 02-named-pipes | ✓WORKS |
| 03-process | ✓5/5 PASSED |
| 04-console-events | ~COMPILES |
| 05-pdcurses | -SKIPPED |
| 06-libevent | -SKIPPED |

## Active Decisions

✓BBC-pattern (wrap, don't rewrite)
✓TDD methodology
✓CMake build system
✓windows/ separate folder
✓4 parallel layers possible

## Blockers

None currently.

## Session End Checklist

```
[ ] Update this file with current state
[ ] Update progress.md with results
[ ] Mark [x] in PORTING-PLAN.md
[ ] Commit changes
```
