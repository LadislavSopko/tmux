# Operational Rules - tmux Windows Port

## Regole OBBLIGATORIE per ogni Task

### 1. Prima di iniziare un task

```
[ ] Aggiorna activeContext.md con:
    - @task::NomeTask
    - ⚡active::DescrizioneBreve
    - Cosa stai per fare
```

### 2. Durante il task

```
[ ] Aggiorna progress.md con:
    - Stato corrente (in corso, bloccato, etc.)
    - Problemi incontrati
    - Decisioni prese
```

### 3. Dopo completato un task

```
[ ] Aggiorna PORTING-PLAN.md - marca [x] i checkbox completati
[ ] Aggiorna progress.md con risultato
[ ] Aggiorna activeContext.md con prossimo step
[ ] COMMIT con messaggio descrittivo
```

---

## Build Commands

### POC (esistenti)

```batch
cd D:\Projekty\AI_Works\tmux\pocs
build.bat
```

Output: `pocs/build/bin/*.exe`

### Windows Port (nuovo)

```batch
cd D:\Projekty\AI_Works\tmux\windows
build.bat
```

Output: `windows/build/bin/*.exe`

### Build manuale (se serve)

```batch
:: Load VS2022/2026 environment
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Configure
cmake -B build -G Ninja

:: Build
cmake --build build

:: Build solo un target
cmake --build build --target test_pty
```

---

## Test Commands

### Eseguire tutti i test

```batch
cd D:\Projekty\AI_Works\tmux\windows
build.bat
ctest --test-dir build --output-on-failure
```

### Eseguire test singolo

```batch
cd D:\Projekty\AI_Works\tmux\windows\build\bin
test_pty.exe
test_ipc.exe
test_proc.exe
test_signal.exe
```

### Test con output verbose

```batch
test_pty.exe --verbose
```

---

## Git Workflow

### Commit per ogni task completato

```bash
# Dopo ogni task completato:
git add windows/
git add memory-bank/activeContext.md
git add memory-bank/progress.md

git commit -m "feat(windows): <descrizione task>

- Cosa è stato fatto
- Test status: PASS/FAIL
- Prossimo step"
```

### Branch strategy

```bash
# Lavoriamo su master per ora
# Se serve feature branch:
git checkout -b windows-port/<feature>
```

### Commit message format

```
feat(windows): implement pty_create() - tests pass

- Added pty-win32.c with CreatePseudoConsole wrapper
- Added test_pty.c with test_pty_create()
- Test status: GREEN
- Next: implement pty_spawn()
```

---

## Memory Bank Updates

### activeContext.md - Template

```markdown
## Current Focus

@task::NomeTaskCorrente
⚡active::DescrizioneBreve

## In Progress

- [ ] Task 1 descrizione
- [ ] Task 2 descrizione

## Blockers

- Nessuno | Lista blockers

## Last Action

Data: YYYY-MM-DD HH:MM
Azione: Cosa è stato fatto
Risultato: OK | FAILED | IN PROGRESS
```

### progress.md - Template per task

```markdown
## Task: NomeTask

[Status] ⚡in-progress | ✓complete | ✗failed

[Started] YYYY-MM-DD
[Completed] YYYY-MM-DD (quando finito)

[Steps]
- [x] Step 1
- [ ] Step 2
- [ ] Step 3

[Issues]
- Issue 1: descrizione + risoluzione

[Result]
Test status: X/Y passed
Commit: <hash>
```

---

## Checklist per Agente

Ogni agente che lavora su un task DEVE:

```
PRIMA:
[ ] Leggere memory-bank/activeContext.md
[ ] Leggere memory-bank/progress.md
[ ] Leggere windows/docs/PORTING-PLAN.md
[ ] Aggiornare activeContext.md con task corrente

DURANTE:
[ ] Scrivere TEST prima dell'implementazione (TDD)
[ ] Eseguire build.bat dopo ogni modifica
[ ] Aggiornare progress.md se ci sono problemi

DOPO:
[ ] Verificare tutti i test PASS
[ ] Aggiornare PORTING-PLAN.md checkbox
[ ] Aggiornare progress.md con risultato
[ ] Aggiornare activeContext.md con prossimo step
[ ] Commit delle modifiche
[ ] Notificare completamento
```

---

## File Locations Quick Reference

| Cosa | Path |
|------|------|
| Memory Bank | `/mnt/d/Projekty/AI_Works/tmux/memory-bank/` |
| Piano | `/mnt/d/Projekty/AI_Works/tmux/windows/docs/PORTING-PLAN.md` |
| TDD Strategy | `/mnt/d/Projekty/AI_Works/tmux/windows/docs/TDD-STRATEGY.md` |
| Build script | `/mnt/d/Projekty/AI_Works/tmux/windows/build.bat` |
| Source | `/mnt/d/Projekty/AI_Works/tmux/windows/src/` |
| Tests | `/mnt/d/Projekty/AI_Works/tmux/windows/tests/` |
| Headers | `/mnt/d/Projekty/AI_Works/tmux/windows/include/` |
| POC reference | `/mnt/d/Projekty/AI_Works/tmux/pocs/` |

---

## Quando Lanciare Agenti in Parallelo

Prerequisiti:
```
[ ] Phase 1 completata (foundation)
[ ] build.bat funziona
[ ] Test framework setup
[ ] Ogni agente ha POC reference assegnato
```

Istruzioni per agente:
```
1. Leggi OPERATIONAL-RULES.md
2. Leggi TDD-STRATEGY.md
3. Leggi il tuo POC reference in pocs/0X-*/
4. Aggiorna activeContext.md
5. Scrivi test PRIMA
6. Implementa fino a GREEN
7. Commit
8. Aggiorna progress.md
```
