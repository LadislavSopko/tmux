# POC 03: Process Management

## Obiettivo

Validare CreateProcess come sostituto di fork()+exec() per gestione processi tmux.

## Requisiti

- Windows 10+
- Visual Studio 2019+ oppure MinGW-w64

## Build

```powershell
cd pocs\03-process
.\build.ps1
```

## Esecuzione

```powershell
.\process_poc.exe
```

## Test da eseguire

1. **Spawn processo** - CreateProcess con cmd e args
2. **Environment** - passare env vars custom
3. **Working directory** - specificare cwd
4. **Wait** - attendere exit (blocking e non-blocking)
5. **Exit code** - ottenere codice di ritorno
6. **Terminate** - terminare processo forzatamente

## Output atteso

```
[+] Process Management POC
[+] Test 1: Spawn and wait...
[+] Created process PID: 1234
[+] Process exited with code: 0

[+] Test 2: Custom environment...
[+] Created process with custom env
[+] Process output shows MY_VAR=hello

[+] Test 3: Non-blocking wait...
[+] Process running...
[+] Polling... not done yet
[+] Polling... done! Exit code: 0

[+] Test 4: Terminate process...
[+] Started long-running process
[+] Terminating...
[+] Process terminated

[+] All tests passed!
```

## Criteri di successo

- [ ] CreateProcess funziona con command line
- [ ] Environment variables passate correttamente
- [ ] Working directory rispettata
- [ ] Wait blocking funziona
- [ ] Wait non-blocking (polling) funziona
- [ ] Exit code recuperato
- [ ] TerminateProcess funziona
