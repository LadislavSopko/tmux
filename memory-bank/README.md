# Memory Bank - tmux Windows Port

§MBEL:5.0
@purpose::AIMemoryEncoding{compression%75,fidelity%100}

## Quick Reference

[Project]
tmux::TerminalMultiplexer{native:Windows}
@goal::Port→Win32+ConPTY{¬WSL,¬MSYS2,¬Cygwin}
@target::Windows10+(1809+)

[Strategy]
BBC::BlackBoxComposition
→WrapPOSIX+Abstractions
→¬TouchCore{cmd-*.c,format.c,grid.c}

## File Structure

```
memory-bank/
├── README.md          (this file - MBEL grammar + structure)
├── productContext.md  (vision, goals, success criteria)
├── systemPatterns.md  (architecture, patterns, components)
├── techContext.md     (stack, dependencies, build commands)
├── activeContext.md   (current task, decisions, blockers)
├── progress.md        (active task progress)
├── history.md         (completed tasks - write only)
└── archive/           (daily archives)
```

## Read Order (IMMUTABLE)

1. productContext.md → Why
2. systemPatterns.md → How
3. techContext.md → What
4. activeContext.md → Now
5. progress.md → Status
6. ¬history.md (write-only)

## MBEL Grammar Reference

See `.ai-agent/src/memory-bank/README.md` for full grammar.

[QuickRef]
> past | @ present | ? future
✓ done | ✗ failed | ! critical | ⚡ active
:: defines | → causes | ← from | + and | - remove
[] section | {} metadata | () note
