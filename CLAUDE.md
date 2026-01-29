# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

# SESSION START PROCEDURE (OBBLIGATORIO)

**Ogni volta che riprendi lavoro su questo progetto, segui ESATTAMENTE questo ordine:**

## Step 1: Memory Bank (capire stato progetto)

```
1. Leggi memory-bank/README.md        → struttura Memory Bank
2. Leggi memory-bank/productContext.md → perché (vision, goals)
3. Leggi memory-bank/systemPatterns.md → come (architettura)
4. Leggi memory-bank/techContext.md    → cosa (stack, dipendenze)
5. Leggi memory-bank/activeContext.md  → ORA (task corrente!)
6. Leggi memory-bank/progress.md       → stato avanzamento
```

## Step 2: Piano di Lavoro (capire cosa fare)

```
7. Leggi windows/docs/PORTING-PLAN.md      → piano con checkbox
8. Leggi windows/docs/OPERATIONAL-RULES.md → regole operative
9. Leggi windows/docs/TDD-STRATEGY.md      → strategia test (se serve)
```

## Step 3: Aggiorna activeContext.md

```
10. Aggiorna activeContext.md con:
    @task::NomeTaskCheStaiPerFare
    ⚡active::DescrizioneBreve
```

## Step 4: Lavora

```
11. Esegui il task seguendo TDD (test prima, poi implementa)
12. Aggiorna progress.md durante il lavoro se ci sono problemi
```

## Step 5: Chiudi il Task

```
13. Marca [x] i checkbox completati in PORTING-PLAN.md
14. Aggiorna progress.md con risultato
15. Aggiorna activeContext.md con prossimo step
16. COMMIT con messaggio descrittivo
```

---

# CODING & INTERACTION NOTES

If you want to ask questions for more spec or other info always use AskUserQuestion tool.

## Collaboration Rules

When working with Claude Code on this project, follow these operational modes and context rules:

### Operational Modes

1. **PLAN Mode**
   - PLAN is "thinking" mode, where Claude discusses implementation details and plans 
   - Default starting mode for all interactions
   - Used for discussing implementation details without making code changes
   - Claude will print `# Mode: PLAN` at the beginning of each response
   - Outputs relevant portions of the plan based on current context level
   - If action is requested, Claude will remind you to approve the plan first

2. **ACT Mode**
   - Only activated when the user explicitly types `ACT`
   - Used for making actual code changes based on the approved plan
   - Claude will print `# Mode: ACT` at the beginning of each response
   - Automatically returns to PLAN mode after each response
   - Can be manually returned to PLAN mode by typing `PLAN`

## Memory Bank - Critical System

The Memory Bank is Claude's ONLY connection to the project between sessions. Without it, Claude starts completely fresh with zero knowledge of the project.

### How Memory Bank Works

1. **User triggers**: Type `mb`, `update memory bank`, or `check memory bank`
2. **Claude's process**:
   - FIRST: Reads `memory-bank/README.md` to understand Memory Bank structure
   - THEN: Reads ALL Memory Bank files to understand current project state
   - FINALLY: Updates relevant files and returns to PLAN mode

### Important Rules

- Claude MUST read memory-bank/README.md first, then ALL Memory Bank files at start of EVERY task
- Memory Bank is the single source of truth - overrides any other documentation
- See memory-bank/README.md for complete Memory Bank documentation

## Project Documentation

Each project has its own README.md with:
- Purpose and architecture
- Build, test, and run commands  
- API usage examples
- Configuration details

Always check the project's README for localized information before working on it.

## Summary of Key Commands

- `mb` or `update memory bank` - Trigger Memory Bank update
- `mcp` - Check MCP server status
- `ACT` - Switch to ACT mode for code changes
- `PLAN` - Return to PLAN mode (default)

## Build Commands (Windows Port)

```batch
:: Build + test tutto
cd D:\Projekty\AI_Works\tmux\windows
build.bat

:: POC esistenti (reference)
cd D:\Projekty\AI_Works\tmux\pocs
build.bat
```

## File Locations

| Cosa | Path |
|------|------|
| Memory Bank | `memory-bank/` |
| Piano lavoro | `windows/docs/PORTING-PLAN.md` |
| Regole operative | `windows/docs/OPERATIONAL-RULES.md` |
| TDD Strategy | `windows/docs/TDD-STRATEGY.md` |
| Build script | `windows/build.bat` |
| Source Windows | `windows/src/` |
| Tests Windows | `windows/tests/` |
| POC reference | `pocs/01-conpty/`, `pocs/02-named-pipes/`, etc. |

## When Spawning Agents (Task tool)

If MCP tools are available (e.g., `mcp__vs-mcp__*`), ALWAYS include in the agent prompt:
> "Use mcp__vs-mcp__* tools instead of Grep/Glob/LS for symbol search and project exploration.
> Use FindSymbols, GetSolutionTree, GetDocumentOutline, FindSymbolUsages instead of file system tools."

This ensures sub-agents also benefit from token-efficient MCP tools.

**CRITICAL for Explore agents:** When using `subagent_type=Explore`, include this instruction so the agent uses VS semantic analysis instead of wasting tokens on file system searches.
