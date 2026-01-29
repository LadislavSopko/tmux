/*
 * POC 05: PDCurses (ncurses replacement)
 *
 * Validates PDCurses as drop-in replacement for ncurses.
 * Target: Windows 10+
 *
 * Build: cl /W4 /Fe:curses_poc.exe curses_poc.c pdcurses.lib user32.lib
 *
 * NOTE: Requires PDCursesMod library and headers.
 * See README.md for setup instructions.
 */

#include <curses.h>
#include <stdlib.h>

/* Color pairs */
#define PAIR_NORMAL    1
#define PAIR_HIGHLIGHT 2
#define PAIR_BORDER    3
#define PAIR_STATUS    4

static int cursor_x = 10;
static int cursor_y = 5;

/*
 * Initialize colors
 */
static void init_colors(void)
{
    if (!has_colors()) {
        return;
    }

    start_color();

    /* Define color pairs */
    init_pair(PAIR_NORMAL, COLOR_WHITE, COLOR_BLACK);
    init_pair(PAIR_HIGHLIGHT, COLOR_BLACK, COLOR_CYAN);
    init_pair(PAIR_BORDER, COLOR_CYAN, COLOR_BLACK);
    init_pair(PAIR_STATUS, COLOR_BLACK, COLOR_GREEN);
}

/*
 * Draw a box using line-drawing characters
 */
static void draw_box(int y, int x, int h, int w)
{
    int i;

    attron(COLOR_PAIR(PAIR_BORDER));

    /* Corners */
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + w - 1, ACS_URCORNER);
    mvaddch(y + h - 1, x, ACS_LLCORNER);
    mvaddch(y + h - 1, x + w - 1, ACS_LRCORNER);

    /* Horizontal lines */
    for (i = 1; i < w - 1; i++) {
        mvaddch(y, x + i, ACS_HLINE);
        mvaddch(y + h - 1, x + i, ACS_HLINE);
    }

    /* Vertical lines */
    for (i = 1; i < h - 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + w - 1, ACS_VLINE);
    }

    attroff(COLOR_PAIR(PAIR_BORDER));
}

/*
 * Draw the UI
 */
static void draw_ui(void)
{
    int max_y, max_x;

    getmaxyx(stdscr, max_y, max_x);

    /* Clear screen */
    erase();

    /* Draw outer box */
    draw_box(0, 0, max_y, max_x);

    /* Title */
    attron(COLOR_PAIR(PAIR_HIGHLIGHT) | A_BOLD);
    mvprintw(1, 2, " PDCurses POC - tmux Windows Port ");
    attroff(COLOR_PAIR(PAIR_HIGHLIGHT) | A_BOLD);

    /* Instructions */
    attron(COLOR_PAIR(PAIR_NORMAL));
    mvprintw(3, 2, "This POC validates PDCurses as ncurses replacement.");
    mvprintw(5, 2, "Features tested:");
    mvprintw(6, 4, "- Box drawing characters (border around this window)");
    mvprintw(7, 4, "- Color support (various colors visible)");
    mvprintw(8, 4, "- Cursor positioning (mvprintw)");
    mvprintw(9, 4, "- Input handling (try arrow keys)");
    mvprintw(10, 4, "- Window resize (try resizing)");
    attroff(COLOR_PAIR(PAIR_NORMAL));

    /* Color demo */
    mvprintw(12, 2, "Color test: ");
    attron(COLOR_PAIR(1)); addstr("Normal "); attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(2)); addstr("Highlight "); attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(3)); addstr("Border "); attroff(COLOR_PAIR(3));
    attron(COLOR_PAIR(4)); addstr("Status"); attroff(COLOR_PAIR(4));

    /* Cursor position indicator */
    mvprintw(14, 2, "Cursor position: (%d, %d)", cursor_x, cursor_y);
    mvprintw(15, 2, "Window size: %d x %d", max_x, max_y);

    /* Instructions */
    attron(COLOR_PAIR(PAIR_STATUS));
    mvprintw(max_y - 2, 2, " Arrow keys: move cursor | r: resize test | q: quit ");
    attroff(COLOR_PAIR(PAIR_STATUS));

    /* Draw cursor marker */
    attron(COLOR_PAIR(PAIR_HIGHLIGHT) | A_BOLD);
    mvaddch(cursor_y, cursor_x, '@');
    attroff(COLOR_PAIR(PAIR_HIGHLIGHT) | A_BOLD);

    refresh();
}

/*
 * Handle input
 */
static int handle_input(void)
{
    int ch;
    int max_y, max_x;

    getmaxyx(stdscr, max_y, max_x);

    ch = getch();

    switch (ch) {
    case 'q':
    case 'Q':
        return 0;  /* Quit */

    case KEY_UP:
        if (cursor_y > 1) cursor_y--;
        break;

    case KEY_DOWN:
        if (cursor_y < max_y - 2) cursor_y++;
        break;

    case KEY_LEFT:
        if (cursor_x > 1) cursor_x--;
        break;

    case KEY_RIGHT:
        if (cursor_x < max_x - 2) cursor_x++;
        break;

    case KEY_RESIZE:
        /* Terminal was resized */
        resize_term(0, 0);  /* Let PDCurses figure out new size */
        break;

    case 'r':
    case 'R':
        /* Manual resize test */
        resize_term(30, 80);
        break;

    default:
        /* Unknown key, show it */
        if (ch != ERR) {
            mvprintw(17, 2, "Key pressed: %d (0x%x)    ", ch, ch);
            refresh();
        }
        break;
    }

    return 1;  /* Continue */
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* Initialize curses */
    initscr();

    if (initscr() == NULL) {
        fprintf(stderr, "[-] initscr() failed\n");
        return 1;
    }

    /* Setup curses modes */
    cbreak();              /* Disable line buffering */
    noecho();              /* Don't echo input */
    keypad(stdscr, TRUE);  /* Enable arrow keys */
    nodelay(stdscr, TRUE); /* Non-blocking input */
    curs_set(0);           /* Hide cursor */

    /* Initialize colors */
    init_colors();

    /* Main loop */
    while (1) {
        draw_ui();

        if (!handle_input()) {
            break;
        }

        /* Small delay to avoid busy loop */
        napms(50);
    }

    /* Cleanup */
    endwin();

    printf("[+] PDCurses POC completed successfully!\n");
    printf("[+] If you saw colors, borders, and cursor movement, it works!\n");

    return 0;
}
