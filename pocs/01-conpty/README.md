# POC 01: ConPTY (Pseudo Console API)

## Obiettivo

Validare che ConPTY può sostituire `forkpty()` per tmux su Windows.

## Requisiti

- Windows 10 versione 1809+ (build 17763+)
- Visual Studio 2019+ oppure MinGW-w64

## Build

### PowerShell + MSVC

```powershell
cd pocs\01-conpty
.\build.ps1
```

### Oppure manualmente

```powershell
cl /W4 /Fe:conpty_poc.exe conpty_poc.c
```

## Esecuzione

```powershell
.\conpty_poc.exe
```

## Test da eseguire

1. **Spawn cmd.exe** - il POC spawna cmd.exe in un PTY
2. **Echo test** - invia "echo hello" e legge output
3. **Resize** - cambia dimensioni PTY durante esecuzione
4. **Exit** - cleanup pulito

## Output atteso

```
[+] Creating pipes...
[+] Creating pseudo console (80x24)...
[+] Starting cmd.exe...
[+] Process started, PID: XXXX
[+] Reading initial output...
Microsoft Windows [Version ...]
(c) Microsoft Corporation. All rights reserved.

C:\...>
[+] Sending command: echo hello
[+] Reading response...
hello

[+] Resizing to 120x40...
[+] Resize successful
[+] Cleanup...
[+] Done!
```

## Criteri di successo

- [ ] PTY creato senza errori
- [ ] cmd.exe spawna e mostra prompt
- [ ] Comando inviato e output ricevuto
- [ ] Resize funziona
- [ ] Nessun memory leak (check con VS diagnostics)

## Note tecniche

- `CreatePseudoConsole()` richiede handle di pipe, non file
- Il processo child va creato con `EXTENDED_STARTUPINFO_PRESENT`
- I/O è asincrono tramite pipe handles
