/*
 * osdep-win32.c - Windows operating system dependent functions
 *
 * Implements the 3 osdep interface functions for Windows:
 * - osdep_get_name()  - Get process name from fd/tty
 * - osdep_get_cwd()   - Get working directory of process
 * - osdep_event_init() - Initialize libevent base
 */

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tmux.h"

/*
 * cur_term - current terminal structure for curses compatibility.
 * PDCurses doesn't use terminfo, so this is a stub.
 * Forward declaration of TERMINAL type (defined in term.h)
 */
struct _terminal;
typedef struct _terminal TERMINAL;
TERMINAL *cur_term = NULL;

/*
 * Get the name of the process associated with a file descriptor.
 * On Windows, we use the console's foreground process or return NULL.
 *
 * This is used by tmux to display the current command in the status bar.
 */
char *
osdep_get_name(int fd, char *tty)
{
    DWORD pid;
    HANDLE hProcess;
    char *name = NULL;
    char path[MAX_PATH];
    char *basename;

    /* Get the console's foreground process */
    if (!GetConsoleProcessList(&pid, 1))
        return NULL;

    /* Open the process to query its name */
    hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess == NULL)
        return NULL;

    /* Get the executable path */
    if (GetModuleFileNameExA(hProcess, NULL, path, sizeof(path)) > 0) {
        /* Extract basename */
        basename = strrchr(path, '\\');
        if (basename != NULL)
            basename++;
        else
            basename = path;

        /* Remove .exe extension if present */
        char *ext = strrchr(basename, '.');
        if (ext != NULL && _stricmp(ext, ".exe") == 0)
            *ext = '\0';

        name = xstrdup(basename);
    }

    CloseHandle(hProcess);
    return name;
}

/*
 * Get the current working directory of a process.
 * On Windows, we query the process's current directory.
 *
 * Returns NULL if the directory cannot be determined.
 */
char *
osdep_get_cwd(int fd)
{
    /*
     * TODO: Implement proper process CWD detection.
     * This requires reading the PEB (Process Environment Block) which
     * is complex. For now, return the current process's directory.
     */
    char *cwd;
    DWORD len;

    len = GetCurrentDirectoryA(0, NULL);
    if (len == 0)
        return NULL;

    cwd = xmalloc(len);
    if (GetCurrentDirectoryA(len, cwd) == 0) {
        free(cwd);
        return NULL;
    }

    return cwd;
}

/*
 * Initialize the libevent event base.
 * On Windows, libevent uses select() or IOCP as the backend.
 *
 * Returns the event base or exits on failure.
 */
struct event_base *
osdep_event_init(void)
{
    struct event_base *base;

    /*
     * On Windows, libevent automatically selects the best backend.
     * We don't need to set any environment variables like on Linux.
     */
    base = event_base_new();
    if (base == NULL)
        fatalx("event_base_new failed");

    return base;
}
