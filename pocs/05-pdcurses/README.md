# POC 05: PDCurses (ncurses Replacement)

## Obiettivo

Validare PDCurses come sostituto drop-in di ncurses per rendering terminale.

## Quale PDCurses?

Ci sono due opzioni:

1. **PDCurses** (originale): https://github.com/wmcbrine/PDCurses
2. **PDCursesMod** (fork attivo): https://github.com/Bill-Gray/PDCursesMod

Consigliato: **PDCursesMod** perché più attivo e con più fix.

## Requisiti

- Windows 10+
- Visual Studio 2019+ oppure MinGW-w64
- PDCursesMod (scaricato e compilato)

## Setup PDCursesMod

```powershell
# Clone PDCursesMod
git clone https://github.com/Bill-Gray/PDCursesMod.git
cd PDCursesMod\wincon

# Build with MSVC
nmake -f Makefile.vc
# Output: pdcurses.lib

# Copy files needed
copy pdcurses.lib ..\..\05-pdcurses\
copy ..\curses.h ..\..\05-pdcurses\
copy ..\panel.h ..\..\05-pdcurses\
```

## Build POC

```powershell
cd pocs\05-pdcurses
.\build.ps1
```

## Esecuzione

```powershell
.\curses_poc.exe
```

## Test da eseguire

1. **Inizializzazione** - initscr, endwin
2. **Output** - mvprintw, refresh
3. **Box drawing** - bordi con caratteri speciali
4. **Colori** - start_color, color pairs
5. **Input** - getch, arrow keys
6. **Resize** - KEY_RESIZE, resize_term

## Output atteso

```
+--------------------------------+
|   PDCurses POC - tmux Port     |
|                                |
|   Colors: [colored text]       |
|                                |
|   Press arrow keys to move     |
|   Press 'q' to quit            |
|                                |
|   Cursor: (10, 5)              |
+--------------------------------+
```

## Criteri di successo

- [ ] initscr/endwin funzionano
- [ ] Box drawing characters corretti
- [ ] Colori funzionanti
- [ ] Arrow keys rilevati
- [ ] Resize gestito
- [ ] No memory leaks

## Note

PDCurses per Windows Console (wincon) è il backend corretto.
Non usare SDL o X11 backend.
