# Active Context - tmux Windows Port

§MBEL:5.0

## Session Start Flow (ALWAYS FOLLOW)

```
1. Read memory-bank/README.md
2. Read memory-bank/*.md (all core files)
3. Check activeContext.md for current task
4. Check progress.md for status
5. Continue work OR ask user what to do
6. Update activeContext+progress BEFORE ending
```

## Current Focus

@task::POC-Validation-Complete!
✓validated::CoreAPIs{ConPTY,NamedPipes,CreateProcess}
?next::PlanImplementation

## Major Milestone Achieved!

!BREAKTHROUGH::CorePOCsWork!
→ConPTY::CanReplace::forkpty()
→NamedPipes::CanReplace::UnixSockets
→CreateProcess::CanReplace::fork+exec

## Build System

[CMake-Based]
✓pocs/CMakeLists.txt
✓pocs/CMakePresets.json
✓pocs/build.bat{loads-VS2026-env}

[Build Command]
```batch
cd D:\Projekty\AI_Works\tmux\pocs
build.bat
```

[Output]
build/bin/Debug/*.exe

## Test Results Summary

[POC-01-ConPTY] ✓WORKS
- CreatePseudoConsole ✓
- Spawn cmd.exe ✓
- Resize ✓

[POC-02-NamedPipes] ✓WORKS
- Server listen ✓
- Client connect ✓
- Bidirectional ✓

[POC-03-Process] ✓5/5-PASSED
- Spawn+wait ✓
- Custom env ✓
- Polling ✓
- Terminate ✓
- Working dir ✓

[POC-04-ConsoleEvents] ~COMPILES
- Needs manual test

[POC-05-PDCurses] -SKIPPED
- Needs dependency

[POC-06-libevent] -SKIPPED
- Needs vcpkg

## Key Discoveries (From Analysis)

[PTY-Layer]
spawn.c:382→fdforkpty()
forkpty-sunos.c→reference
!Windows::ConPTY-validated✓

[IPC-Layer]
server.c:106→server_create_socket()
AF_UNIX+SOCK_STREAM
!Windows::NamedPipes-validated✓

[Signal-Layer]
server.c:430→server_signal()
SIGCHLD→waitpid()
?Windows::ConsoleEvents{needs-manual-test}

[Process-Layer]
job.c:120→fork()
!Windows::CreateProcess-validated✓

## Active Decisions

✓BBC-pattern{wrap-not-rewrite}
✓ConPTY{Win10-1809+}
✓NamedPipes{IPC}
✓CMake{build-system}
✓POC-First{validate-before-implement}!

## Next Steps

?Phase1::CreateImplementationPlan
→Identify::AllFilesToModify
→Design::AbstractionLayers
→Estimate::Effort
→Prioritize::Tasks

## Session End Checklist

```
✓ Update activeContext.md with current state
✓ Update progress.md with completed/pending
✓ Update history.md with session summary
□ Commit if significant progress
```
