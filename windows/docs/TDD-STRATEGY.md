# TDD Strategy - tmux Windows Port

## Principio Base

I **POC esistenti sono i nostri test di riferimento**. Ogni POC dimostra che un'API Windows funziona.

```
POC (Test) → Implementazione → Integrazione con tmux
```

## Mapping POC → Layer → Test

| POC | Cosa testa | Layer Win32 | Test Strategy |
|-----|------------|-------------|---------------|
| `01-conpty` | CreatePseudoConsole, spawn, resize | pty-win32 | POC = acceptance test |
| `02-named-pipes` | CreateNamedPipe, connect, send/recv | ipc-win32 | POC = acceptance test |
| `03-process` | CreateProcess, wait, terminate, env | proc-win32 | POC = acceptance test (5 test già passati!) |
| `04-console-events` | SetConsoleCtrlHandler, events | signal-win32 | POC = acceptance test |

## TDD Flow per Ogni Layer

### 1. PTY Layer (pty-win32)

**Test esistente:** `pocs/01-conpty/`
- ✓ CreatePseudoConsole(80x24) funziona
- ✓ Spawn cmd.exe funziona
- ✓ Read/Write funziona
- ✓ Resize funziona

**TDD Steps:**
```
[ ] 1. Crea test unitario: test_pty_create()
       - Input: cols=80, rows=24
       - Expected: handle valido, no errori
       - Basato su POC linea dove crea console

[ ] 2. Implementa pty_create() fino a test PASS

[ ] 3. Crea test: test_pty_spawn()
       - Input: cmd="cmd.exe"
       - Expected: pid > 0, processo attivo

[ ] 4. Implementa pty_spawn() fino a test PASS

[ ] 5. Crea test: test_pty_read_write()
       - Write "echo hello\r\n"
       - Read output contiene "hello"

[ ] 6. Implementa pty_read/write fino a test PASS

[ ] 7. Crea test: test_pty_resize()
       - Resize a 120x40
       - Expected: no errori

[ ] 8. Implementa pty_resize() fino a test PASS

[ ] 9. Integration test: spawn cmd.exe, send command, verify output
```

### 2. IPC Layer (ipc-win32)

**Test esistente:** `pocs/02-named-pipes/`
- ✓ Server listen funziona
- ✓ Client connect funziona
- ✓ Bidirectional send/recv funziona

**TDD Steps:**
```
[ ] 1. Crea test: test_ipc_listen()
       - Crea pipe \\.\pipe\tmux-test
       - Expected: handle valido

[ ] 2. Implementa ipc_listen() fino a test PASS

[ ] 3. Crea test: test_ipc_connect()
       - Connetti a pipe esistente
       - Expected: handle valido

[ ] 4. Implementa ipc_connect() fino a test PASS

[ ] 5. Crea test: test_ipc_send_recv()
       - Server sends "hello"
       - Client receives "hello"
       - Client sends "world"
       - Server receives "world"

[ ] 6. Implementa ipc_send/recv fino a test PASS

[ ] 7. Integration test: client-server full duplex
```

### 3. Process Layer (proc-win32)

**Test esistente:** `pocs/03-process/` - **5/5 TESTS PASSED!**
- ✓ Test1: Spawn+Wait (exit code 0)
- ✓ Test2: Custom env (MY_VAR=hello)
- ✓ Test3: Non-blocking poll
- ✓ Test4: Terminate process
- ✓ Test5: Working directory

**TDD Steps:**
```
[ ] 1. Crea test: test_proc_spawn_wait()
       - Spawn "cmd /c exit 0"
       - Wait, expect exit code 0
       - ALREADY PROVEN IN POC!

[ ] 2. Implementa proc_spawn/wait - copia da POC

[ ] 3. Crea test: test_proc_spawn_env()
       - Set MY_VAR=test
       - Spawn "cmd /c echo %MY_VAR%"
       - Verify output contains "test"

[ ] 4. Implementa proc_spawn con env - copia da POC

[ ] 5. Crea test: test_proc_kill()
       - Spawn long-running process
       - Kill it
       - Verify terminated

[ ] 6. Implementa proc_kill - copia da POC

[ ] 7. Integration test: full process lifecycle
```

### 4. Signal Layer (signal-win32)

**Test esistente:** `pocs/04-console-events/`
- ~ Compiles, needs manual test

**TDD Steps:**
```
[ ] 1. Test manuale: verifica POC-04 funziona

[ ] 2. Crea test: test_signal_ctrl_c()
       - Setup handler per SIGINT
       - Genera CTRL_C_EVENT
       - Verifica handler chiamato

[ ] 3. Implementa win32_signal(SIGINT, handler)

[ ] 4. Crea test: test_signal_child_exit()
       - Spawn processo
       - Aspetta terminazione
       - Verifica "SIGCHLD" emulato

[ ] 5. Implementa child monitoring con WaitForSingleObject

[ ] 6. Integration test: graceful shutdown
```

## Test Framework

Per i test unitari Windows, usa:
- **Opzione A:** Test semplici con assert() come nei POC
- **Opzione B:** Unity (C test framework, single header)
- **Opzione C:** CMocka

**Raccomandazione:** Inizia con assert() come nei POC, poi migra se serve.

## Struttura File Test

```
windows/
├── src/
│   ├── pty-win32.c
│   ├── ipc-win32.c
│   ├── proc-win32.c
│   └── signal-win32.c
├── tests/
│   ├── test_pty.c        # Test per pty-win32
│   ├── test_ipc.c        # Test per ipc-win32
│   ├── test_proc.c       # Test per proc-win32
│   ├── test_signal.c     # Test per signal-win32
│   └── test_main.c       # Runner
└── CMakeLists.txt        # Include test target
```

## Parallelizzazione con TDD

**4 Agenti possono lavorare in parallelo SE:**
1. Ogni agente ha il suo POC come riferimento
2. Ogni agente scrive test PRIMA dell'implementazione
3. I test sono indipendenti (non condividono stato)

| Agente | POC Riferimento | Output |
|--------|-----------------|--------|
| Agent-PTY | pocs/01-conpty | pty-win32.c + test_pty.c |
| Agent-IPC | pocs/02-named-pipes | ipc-win32.c + test_ipc.c |
| Agent-PROC | pocs/03-process | proc-win32.c + test_proc.c |
| Agent-SIGNAL | pocs/04-console-events | signal-win32.c + test_signal.c |

## Ordine Dipendenze Post-TDD

Dopo che i 4 layer passano i test:

```
[ ] daemon-win32.c (usa proc-win32) → test_daemon.c
[ ] imsg-win32.c (usa ipc-win32) → test_imsg.c
[ ] Adapt spawn.c (usa pty-win32) → integration test
[ ] Adapt server.c (usa ipc-win32) → integration test
[ ] Adapt client.c (usa ipc-win32) → integration test
```

## Checklist TDD Compliance

Per ogni file:
- [ ] Test scritto PRIMA dell'implementazione
- [ ] Test fallisce inizialmente (RED)
- [ ] Implementazione minima per far passare (GREEN)
- [ ] Refactor se necessario
- [ ] Test passa in CI

---

*Nota: I POC in pocs/ sono la nostra "golden reference" - il codice che sappiamo funziona.*
