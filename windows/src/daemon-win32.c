/* daemon-win32.c - Background process for Windows */

#include "compat-win32.h"

/*
 * daemon - detach from controlling terminal and run in background
 *
 * On Windows, this is a stub. True daemonization would require
 * respawning the process with CREATE_NO_WINDOW flag.
 * For now, just return success.
 */
int
daemon(int nochdir, int noclose)
{
    (void)nochdir;
    (void)noclose;
    /* TODO: Implement proper Windows service/detach behavior */
    return 0;
}

/*
 * win32_daemon - Windows-specific daemon implementation
 */
int
win32_daemon(int nochdir, int noclose)
{
    return daemon(nochdir, noclose);
}
