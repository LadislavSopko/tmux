# POC 02: Named Pipes (IPC Replacement)

## Obiettivo

Validare Named Pipes come sostituto di Unix Domain Sockets per IPC client/server tmux.

## Requisiti

- Windows 10+
- Visual Studio 2019+ oppure MinGW-w64

## Build

### PowerShell + MSVC

```powershell
cd pocs\02-named-pipes
.\build.ps1
```

## Esecuzione

### Terminal 1 (Server)

```powershell
.\pipe_server.exe
```

### Terminal 2 (Client)

```powershell
.\pipe_client.exe
```

## Test da eseguire

1. **Server listen** - server crea named pipe e aspetta
2. **Client connect** - client si connette
3. **Bidirezionale** - messaggi in entrambe le direzioni
4. **Multiple clients** - avviare più client

## Output atteso

### Server

```
[+] Named Pipes Server POC
[+] Creating pipe: \\.\pipe\tmux-test
[+] Waiting for client...
[+] Client connected!
[+] Received: Hello from client
[+] Sent response: Hello from server
[+] Client disconnected
[+] Waiting for next client...
```

### Client

```
[+] Named Pipes Client POC
[+] Connecting to: \\.\pipe\tmux-test
[+] Connected!
[+] Sending: Hello from client
[+] Received: Hello from server
[+] Done!
```

## Criteri di successo

- [ ] Server crea pipe e ascolta
- [ ] Client si connette
- [ ] Messaggi bidirezionali funzionano
- [ ] Multiple clients possono connettersi (sequenzialmente)
- [ ] Overlapped I/O funziona (non-blocking)

## Path Convention

```
POSIX:   /tmp/tmux-{uid}/default
Windows: \\.\pipe\tmux-{username}-default
```
