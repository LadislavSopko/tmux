# POCs - tmux Windows Port

Proof of Concepts per validare le tecnologie Windows prima dell'implementazione.

## Strategia

```
POC → Validate → Learn → Plan → Implement
```

Ogni POC deve essere:
- **Standalone** - compilabile ed eseguibile indipendentemente
- **Minimale** - solo il codice necessario per validare
- **Documentato** - cosa funziona, cosa no, problemi trovati

---

## POC Status

| # | POC | Priorità | Status | Note |
|---|-----|----------|--------|------|
| 01 | ConPTY | **ALTA** | ⏳ TODO | Core del port |
| 02 | Named Pipes | **ALTA** | ⏳ TODO | IPC client/server |
| 03 | Process | MEDIA | ⏳ TODO | CreateProcess wrapper |
| 04 | Console Events | MEDIA | ⏳ TODO | Signal replacement |
| 05 | PDCurses | MEDIA | ⏳ TODO | ncurses replacement |
| 06 | libevent-win | BASSA | ⏳ TODO | Già supportato, verifica |

---

## 01-conpty: Pseudo Console API

**Obiettivo**: Validare che ConPTY può sostituire forkpty() per tmux.

### Cosa testare

1. **Creazione PTY**
   - `CreatePseudoConsole()` con dimensioni specifiche
   - Ottenere handle input/output

2. **Spawn processo**
   - `CreateProcess()` con `STARTUPINFOEX`
   - Collegare processo al PTY
   - Testare con `cmd.exe`, `powershell.exe`

3. **I/O bidirezionale**
   - Scrivere comandi al PTY
   - Leggere output dal PTY
   - Gestire buffering

4. **Resize**
   - `ResizePseudoConsole()` durante esecuzione
   - Verificare che il processo riceva la notifica

5. **Cleanup**
   - `ClosePseudoConsole()`
   - Terminazione corretta del processo child

### Criteri di successo

```
✓ Spawn cmd.exe interattivo
✓ Eseguire "echo hello" e leggere output
✓ Resize da 80x24 a 120x40
✓ Exit pulito senza leak
```

### Dipendenze

- Windows 10 1809+ (build 17763+)
- `#include <windows.h>`
- `#include <consoleapi.h>`

### File da creare

```
01-conpty/
├── README.md       (istruzioni specifiche)
├── conpty_poc.c    (implementazione)
├── build.bat       (compilazione MSVC)
└── build.sh        (compilazione MinGW cross)
```

---

## 02-named-pipes: IPC Replacement

**Obiettivo**: Validare Named Pipes come sostituto di Unix Domain Sockets.

### Cosa testare

1. **Server**
   - `CreateNamedPipe()` con `PIPE_ACCESS_DUPLEX`
   - `FILE_FLAG_OVERLAPPED` per async I/O
   - `ConnectNamedPipe()` per accept

2. **Client**
   - `CreateFile()` per connessione
   - `FILE_FLAG_OVERLAPPED`

3. **Comunicazione**
   - Invio/ricezione messaggi
   - Protocollo binario (come tmux usa)
   - Test latenza

4. **Multiple clients**
   - Server accetta N client
   - Gestione concorrente

5. **Overlapped I/O**
   - Non-blocking operations
   - Integrazione con event loop

### Criteri di successo

```
✓ Server listen su \\.\pipe\tmux-test
✓ Client connect
✓ Echo message bidirezionale
✓ 3+ client simultanei
✓ Overlapped I/O funzionante
```

### Path convention

```
POSIX:   /tmp/tmux-{uid}/default
Windows: \\.\pipe\tmux-{username}-default
```

### File da creare

```
02-named-pipes/
├── README.md
├── pipe_server.c
├── pipe_client.c
├── build.bat
└── build.sh
```

---

## 03-process: Process Management

**Obiettivo**: Validare CreateProcess come sostituto di fork()+exec().

### Cosa testare

1. **Spawn base**
   - `CreateProcess()` con command line
   - Working directory
   - Environment variables

2. **Wait**
   - `WaitForSingleObject()` blocking
   - `WaitForSingleObject(0)` non-blocking (polling)
   - `WaitForMultipleObjects()` per più processi

3. **Termination**
   - `TerminateProcess()` (equivalente SIGKILL)
   - Graceful shutdown (come?)
   - Exit code retrieval

4. **Job Objects** (opzionale)
   - Raggruppare processi
   - Kill gruppo intero

### Criteri di successo

```
✓ Spawn processo con env custom
✓ Wait for exit
✓ Get exit code
✓ Terminate running process
```

### File da creare

```
03-process/
├── README.md
├── process_poc.c
├── build.bat
└── build.sh
```

---

## 04-console-events: Signal Replacement

**Obiettivo**: Validare gestione eventi console come sostituto di POSIX signals.

### Mapping signals → Windows

| Signal | Uso tmux | Windows Equivalent |
|--------|----------|-------------------|
| SIGCHLD | Child exit | WaitForSingleObject + thread |
| SIGTERM | Shutdown | SetConsoleCtrlHandler |
| SIGINT | Ctrl+C | CTRL_C_EVENT |
| SIGWINCH | Resize | GetConsoleScreenBufferInfo polling |
| SIGHUP | Hangup | CTRL_CLOSE_EVENT |

### Cosa testare

1. **Console Control Handler**
   - `SetConsoleCtrlHandler()`
   - CTRL_C_EVENT, CTRL_BREAK_EVENT
   - CTRL_CLOSE_EVENT

2. **Child monitoring**
   - Thread dedicato con `WaitForSingleObject()`
   - Callback su child exit
   - Multiple children

3. **Resize detection**
   - Polling `GetConsoleScreenBufferInfo()`
   - O evento da Windows Terminal?

### Criteri di successo

```
✓ Catch Ctrl+C senza terminare
✓ Detect child exit async
✓ Detect console resize
```

### File da creare

```
04-console-events/
├── README.md
├── events_poc.c
├── build.bat
└── build.sh
```

---

## 05-pdcurses: Terminal UI

**Obiettivo**: Validare PDCurses come sostituto di ncurses.

### Cosa testare

1. **Inizializzazione**
   - `initscr()`, `endwin()`
   - Raw mode, noecho

2. **Rendering**
   - `mvprintw()`, `refresh()`
   - Box drawing characters
   - UTF-8 support?

3. **Colori**
   - `start_color()`
   - 256 colors support?
   - True color?

4. **Input**
   - `getch()`, `wgetch()`
   - Special keys (arrows, function keys)
   - Mouse?

5. **Resize**
   - `resize_term()`
   - `KEY_RESIZE`

### Criteri di successo

```
✓ Draw box with borders
✓ Colors working
✓ Arrow keys detected
✓ Resize handled
```

### Quale PDCurses?

- **PDCurses** (wmcbrine): https://github.com/wmcbrine/PDCurses
- **PDCursesMod** (Bill-Gray): https://github.com/Bill-Gray/PDCursesMod (più attivo)

### File da creare

```
05-pdcurses/
├── README.md
├── curses_poc.c
├── build.bat
└── build.sh
```

---

## 06-libevent-win: Event Loop

**Obiettivo**: Verificare che libevent funzioni su Windows con i nostri handle.

### Cosa testare

1. **Base**
   - `event_base_new()`
   - Loop funzionante

2. **Timer**
   - `evtimer_set()`, `evtimer_add()`

3. **I/O events**
   - Named Pipe handles
   - Console handles (se possibile)

4. **Integrazione**
   - ConPTY pipe + libevent
   - Named Pipe IPC + libevent

### Criteri di successo

```
✓ Event loop runs
✓ Timer fires
✓ Pipe I/O events work
```

### Note

libevent già supporta Windows, ma dobbiamo verificare che funzioni con:
- Named Pipes (non solo sockets)
- ConPTY handles

### File da creare

```
06-libevent-win/
├── README.md
├── libevent_poc.c
├── build.bat
└── build.sh
```

---

## Ordine di Esecuzione

```
01-conpty ──────┐
                ├──→ 06-libevent-win ──→ INTEGRATION
02-named-pipes ─┤
                │
03-process ─────┤
                │
04-console-events
                │
05-pdcurses ────┘
```

**Priorità**:
1. `01-conpty` - senza questo, niente funziona
2. `02-named-pipes` - necessario per client/server
3. `03-process` + `04-console-events` - in parallelo
4. `05-pdcurses` - può essere testato indipendentemente
5. `06-libevent-win` - integrazione finale

---

## Build Environment

### MSVC (raccomandato)

```batch
cl /W4 /Fe:poc.exe poc.c
```

### MinGW (cross-compile da Linux)

```bash
x86_64-w64-mingw32-gcc -o poc.exe poc.c
```

### Dipendenze comuni

```c
#define _WIN32_WINNT 0x0A00  // Windows 10+
#include <windows.h>
```

---

## Dopo i POC

Una volta completati tutti i POC con successo:

1. **Documentare findings** - cosa funziona, workaround trovati
2. **Creare piano implementazione** - basato su ciò che abbiamo imparato
3. **Stimare effort** - ora sappiamo cosa serve davvero
4. **Iniziare implementazione** - con confidenza
