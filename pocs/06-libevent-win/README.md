# POC 06: libevent on Windows

## Obiettivo

Verificare che libevent funzioni su Windows con gli handle che useremo (pipes, timers).

## Note

libevent supporta già Windows nativamente. Questo POC verifica che:
1. L'event loop funziona
2. I timer funzionano
3. Gli I/O events su pipe funzionano
4. Possiamo integrare con Named Pipes

## Requisiti

- Windows 10+
- Visual Studio 2019+ oppure MinGW-w64
- libevent (da vcpkg o build manuale)

## Setup libevent

### Opzione 1: vcpkg (consigliato)

```powershell
# Install vcpkg if not present
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install libevent
.\vcpkg install libevent:x64-windows
.\vcpkg integrate install
```

### Opzione 2: Build manuale

```powershell
git clone https://github.com/libevent/libevent.git
cd libevent
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Build POC

```powershell
cd pocs\06-libevent-win
.\build.ps1
```

## Esecuzione

```powershell
.\libevent_poc.exe
```

## Test da eseguire

1. **Event loop** - event_base_new, event_base_dispatch
2. **Timer** - evtimer_set, evtimer_add
3. **Signal-like** - evsignal (se supportato su Windows)
4. **I/O** - read/write events su pipe handle

## Output atteso

```
[+] libevent POC - tmux Windows Port
[+] libevent version: 2.1.12-stable

[+] Creating event base...
[+] Using backend: win32

[+] Setting up timer (fires every 1 second)...
[+] Setting up pipe I/O...

[+] Starting event loop...
[!] Timer fired! Count: 1
[!] Timer fired! Count: 2
[!] Pipe read event triggered
[!] Timer fired! Count: 3
...
[+] Test complete after 5 timer events
```

## Criteri di successo

- [ ] event_base_new() funziona
- [ ] Timer events funzionano
- [ ] Pipe I/O events funzionano
- [ ] Loop termina correttamente
- [ ] Nessun memory leak

## Note tecniche

- Su Windows libevent usa IOCP o select backend
- Named Pipes potrebbero richiedere wrapper per integrazione
- Verificare compatibilità con overlapped I/O
