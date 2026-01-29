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
?CrossPlatformBuild{CMake}

## Success Criteria

[Milestone1::PTYWorks]
?ConPTY::Create+Spawn+Read+Write✓
?Test::cmd.exe{interactive}

[Milestone2::IPCWorks]
?NamedPipes::Listen+Connect+Send+Recv✓
?Test::Client↔Server{attach,detach}

[Milestone3::ProcessWorks]
?CreateProcess::Spawn+Wait+Kill✓
?Test::Jobs{background,foreground}

[Milestone4::SignalsWork]
?ConsoleEvents::Ctrl-C+ChildExit✓
?Test::Graceful{shutdown,resize}

[Milestone5::Integration]
?tmux::new-session+split-pane+detach+attach✓
?FullFunctionality

## Non-Goals

¬Rewrite::Core{cmd-*.c,format.c,grid.c,layout.c}
¬Support::OlderWindows{<10-1809}
¬GUI::ThisIsTerminalApp
