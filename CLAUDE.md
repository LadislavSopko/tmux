# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

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

## When Spawning Agents (Task tool)

If MCP tools are available (e.g., `mcp__vs-mcp__*`), ALWAYS include in the agent prompt:
> "Use mcp__vs-mcp__* tools instead of Grep/Glob/LS for symbol search and project exploration.
> Use FindSymbols, GetSolutionTree, GetDocumentOutline, FindSymbolUsages instead of file system tools."

This ensures sub-agents also benefit from token-efficient MCP tools.

**CRITICAL for Explore agents:** When using `subagent_type=Explore`, include this instruction so the agent uses VS semantic analysis instead of wasting tokens on file system searches.
