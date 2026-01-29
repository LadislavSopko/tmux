/*
 * POC 04: Console Events (Signal Replacement)
 *
 * Validates console event handling as replacement for POSIX signals.
 * Target: Windows 10+
 *
 * Build: cl /W4 /Fe:events_poc.exe events_poc.c
 */

#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>

/* Global state */
static volatile int g_running = 1;
static volatile int g_ctrl_c_count = 0;
static HANDLE g_child_process = NULL;
static COORD g_last_size = {0, 0};

/*
 * Console control handler (like signal handler)
 */
static BOOL WINAPI console_handler(DWORD event)
{
    switch (event) {
    case CTRL_C_EVENT:
        g_ctrl_c_count++;
        printf("\n[!] Caught CTRL_C_EVENT #%d - handled!\n", g_ctrl_c_count);
        printf("[+] (Press Ctrl+Break to exit)\n");
        return TRUE;  /* Handled, don't terminate */

    case CTRL_BREAK_EVENT:
        printf("\n[!] Caught CTRL_BREAK_EVENT - exiting gracefully\n");
        g_running = 0;
        return TRUE;

    case CTRL_CLOSE_EVENT:
        printf("\n[!] Caught CTRL_CLOSE_EVENT (console closing)\n");
        g_running = 0;
        return TRUE;

    case CTRL_LOGOFF_EVENT:
        printf("\n[!] Caught CTRL_LOGOFF_EVENT\n");
        return FALSE;  /* Let system handle */

    case CTRL_SHUTDOWN_EVENT:
        printf("\n[!] Caught CTRL_SHUTDOWN_EVENT\n");
        return FALSE;  /* Let system handle */

    default:
        return FALSE;
    }
}

/*
 * Child process monitor thread (simulates SIGCHLD)
 */
static DWORD WINAPI child_monitor_thread(LPVOID param)
{
    HANDLE hProcess = (HANDLE)param;
    DWORD exitCode;

    printf("[+] Child monitor thread started\n");

    /* Wait for child to exit */
    WaitForSingleObject(hProcess, INFINITE);

    /* Get exit code */
    GetExitCodeProcess(hProcess, &exitCode);
    printf("[+] Child process exited with code: %lu (SIGCHLD equivalent)\n", exitCode);

    CloseHandle(hProcess);
    g_child_process = NULL;

    return 0;
}

/*
 * Spawn a child process and start monitoring thread
 */
static int spawn_monitored_child(void)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    HANDLE hThread;
    char cmdline[] = "cmd.exe /c ping -n 5 127.0.0.1 >nul";

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "[-] CreateProcess failed: %lu\n", GetLastError());
        return -1;
    }

    printf("[+] Spawned test child PID: %lu (will exit in ~5 seconds)\n", pi.dwProcessId);
    CloseHandle(pi.hThread);

    g_child_process = pi.hProcess;

    /* Start monitor thread */
    hThread = CreateThread(NULL, 0, child_monitor_thread, pi.hProcess, 0, NULL);
    if (!hThread) {
        fprintf(stderr, "[-] CreateThread failed: %lu\n", GetLastError());
        CloseHandle(pi.hProcess);
        return -1;
    }
    CloseHandle(hThread);  /* Don't need handle, thread runs independently */

    return 0;
}

/*
 * Get current console size
 */
static int get_console_size(COORD *size)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return -1;
    }

    size->X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    size->Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return 0;
}

/*
 * Check for console resize (simulates SIGWINCH)
 */
static void check_console_resize(void)
{
    COORD current;

    if (get_console_size(&current) < 0) {
        return;
    }

    if (g_last_size.X == 0) {
        /* First call, just save size */
        g_last_size = current;
        return;
    }

    if (current.X != g_last_size.X || current.Y != g_last_size.Y) {
        printf("[+] Console resized: %dx%d -> %dx%d (SIGWINCH equivalent)\n",
               g_last_size.X, g_last_size.Y, current.X, current.Y);
        g_last_size = current;
    }
}

int main(int argc, char *argv[])
{
    int poll_count = 0;

    (void)argc;
    (void)argv;

    printf("[+] Console Events POC - tmux Windows Port\n");
    printf("[+] Testing: Ctrl+C handling, child monitoring, resize detection\n\n");

    /* Install console handler */
    if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
        fprintf(stderr, "[-] SetConsoleCtrlHandler failed: %lu\n", GetLastError());
        return 1;
    }
    printf("[+] Console handler installed\n");

    /* Get initial console size */
    get_console_size(&g_last_size);
    printf("[+] Console size: %dx%d\n", g_last_size.X, g_last_size.Y);

    /* Spawn a child process with monitoring */
    printf("\n[+] Spawning monitored child process...\n");
    if (spawn_monitored_child() < 0) {
        printf("[-] Failed to spawn child (continuing anyway)\n");
    }

    printf("\n[+] Event loop running. Try:\n");
    printf("    - Press Ctrl+C (will be caught)\n");
    printf("    - Resize the console window\n");
    printf("    - Wait for child process to exit\n");
    printf("    - Press Ctrl+Break to exit\n\n");

    /* Main loop - polling for resize, handling events */
    while (g_running) {
        /* Check for resize every second */
        check_console_resize();

        /* Poll sleep */
        Sleep(1000);
        poll_count++;

        /* Status update every 5 seconds */
        if (poll_count % 5 == 0) {
            printf("[.] Still running... (Ctrl+C count: %d, child: %s)\n",
                   g_ctrl_c_count,
                   g_child_process ? "running" : "exited");
        }
    }

    /* Cleanup */
    if (g_child_process) {
        printf("[+] Terminating remaining child process...\n");
        TerminateProcess(g_child_process, 0);
        CloseHandle(g_child_process);
    }

    printf("[+] Exiting cleanly\n");
    printf("[+] Total Ctrl+C events caught: %d\n", g_ctrl_c_count);

    return 0;
}
