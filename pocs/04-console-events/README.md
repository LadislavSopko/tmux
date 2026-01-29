# POC 04: Console Events (Signal Replacement)

## Obiettivo

Validare la gestione degli eventi console come sostituto di POSIX signals.

## Mapping Signals → Windows

| Signal | Uso in tmux | Windows Equivalent |
|--------|-------------|-------------------|
| SIGINT | Ctrl+C | CTRL_C_EVENT |
| SIGTERM | Shutdown | CTRL_CLOSE_EVENT |
| SIGCHLD | Child exit | WaitForSingleObject thread |
| SIGWINCH | Resize | Console API polling |
| SIGHUP | Hangup | CTRL_CLOSE_EVENT |

## Requisiti

- Windows 10+
- Visual Studio 2019+ oppure MinGW-w64

## Build

```powershell
cd pocs\04-console-events
.\build.ps1
```

## Esecuzione

```powershell
.\events_poc.exe
```

## Test da eseguire

1. **Ctrl+C handling** - catturare senza terminare
2. **Child exit monitoring** - thread che monitora processi child
3. **Console resize detection** - polling dimensioni console

## Output atteso

```
[+] Console Events POC
[+] Press Ctrl+C to test (will be caught, not terminate)
[+] Press Ctrl+Break to exit

[+] Starting child process monitor...
[+] Spawned test child PID: 1234

[+] Waiting for events...
[!] Caught CTRL_C_EVENT - handled!
[!] Caught CTRL_C_EVENT - handled!
[+] Child process 1234 exited
[+] Console resized: 120x40
[!] Caught CTRL_BREAK_EVENT - exiting
```

## Criteri di successo

- [ ] Ctrl+C catturato senza terminare
- [ ] Ctrl+Break termina pulitamente
- [ ] Thread monitor rileva child exit
- [ ] Resize detection funziona
