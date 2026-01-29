# Product Context - tmux Windows Port

§MBEL:5.0

## Vision

@vision::tmux{native:Windows,¬emulation}
→BringUnixPower→WindowsUsers
→FirstClass+Professional+Fast

## Problem Statement

[CurrentState]
tmux::Unix-only{POSIX-dependent}
WindowsUsers→WSL|MSYS2|Cygwin{overhead+friction}
¬NativeExperience

[TargetState]
tmux.exe::Native{ConPTY+Win32API}
→ZeroEmulation
→DirectIntegration{WindowsTerminal,cmd,PowerShell}

## Goals

[Primary]
!Port::tmux→Windows{native}
@preserve::AllFeatures{sessions,windows,panes,keybindings}
@target::Windows10+{1809+,ConPTY}

[Secondary]
?MaintainUpstreamCompat{merge-friendly}
?MinimalCoreChanges{BBC-pattern}
✓CrossPlatformBuild{CMake}

## Success Criteria

[Milestone1::PTYWorks] ✓VALIDATED
✓ConPTY::Create+Spawn+Read+Write
✓Test::cmd.exe{interactive}
→POC-01-conpty::WORKING

[Milestone2::IPCWorks] ✓VALIDATED
✓NamedPipes::Listen+Connect+Send+Recv
✓Test::Client↔Server{bidirectional}
→POC-02-named-pipes::WORKING

[Milestone3::ProcessWorks] ✓VALIDATED
✓CreateProcess::Spawn+Wait+Kill
✓Test::5/5-PASSED{env,poll,terminate,cwd}
→POC-03-process::WORKING

[Milestone4::SignalsWork] ~PARTIAL
~ConsoleEvents::Compiles
?Test::Needs-manual-validation
→POC-04-console-events::COMPILES

[Milestone5::Phase1Foundation] ✓COMPLETE
✓tmux.exe::Compiles+Links
✓154/154-files::Compile
✓Runtime::tmux-V-works
→Phase1::100%-DONE

[Milestone6::Integration] ?PENDING
?tmux::new-session+split-pane+detach+attach
?FullFunctionality
→Requires::Phase2-6-implementation

## Non-Goals

¬Rewrite::Core{cmd-*.c,format.c,grid.c,layout.c}
¬Support::OlderWindows{<10-1809}
¬GUI::ThisIsTerminalApp
