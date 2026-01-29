/*
 * POC 01: ConPTY - Pseudo Console API
 *
 * Validates ConPTY as replacement for forkpty() on Windows.
 * Target: Windows 10 1809+ (build 17763+)
 *
 * Build: cl /W4 /Fe:conpty_poc.exe conpty_poc.c
 */

#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <consoleapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ConPTY handle and associated pipes */
typedef struct {
    HPCON hPC;              /* Pseudo console handle */
    HANDLE hPipeIn;         /* Pipe: PTY -> Us (read from PTY) */
    HANDLE hPipeOut;        /* Pipe: Us -> PTY (write to PTY) */
    HANDLE hProcess;        /* Child process handle */
    HANDLE hThread;         /* Child main thread handle */
    DWORD dwProcessId;      /* Child PID */
} ConPTY;

/* Forward declarations */
static int create_pipes(HANDLE *phPipeIn, HANDLE *phPipeOut,
                        HANDLE *phPipePTYIn, HANDLE *phPipePTYOut);
static int create_pseudo_console(ConPTY *pty, HANDLE hPipePTYIn,
                                  HANDLE hPipePTYOut, int cols, int rows);
static int spawn_process(ConPTY *pty, const char *cmdline);
static int pty_read(ConPTY *pty, char *buf, size_t buflen, DWORD *bytesRead);
static int pty_write(ConPTY *pty, const char *buf, size_t len);
static int pty_resize(ConPTY *pty, int cols, int rows);
static void pty_cleanup(ConPTY *pty);

/*
 * Create pipe pairs for PTY communication
 */
static int create_pipes(HANDLE *phPipeIn, HANDLE *phPipeOut,
                        HANDLE *phPipePTYIn, HANDLE *phPipePTYOut)
{
    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;  /* Pipes must be inheritable */

    /* Pipe for reading from PTY (PTY stdout -> our stdin) */
    if (!CreatePipe(phPipeIn, phPipePTYOut, &sa, 0)) {
        fprintf(stderr, "[-] CreatePipe (read) failed: %lu\n", GetLastError());
        return -1;
    }

    /* Pipe for writing to PTY (our stdout -> PTY stdin) */
    if (!CreatePipe(phPipePTYIn, phPipeOut, &sa, 0)) {
        fprintf(stderr, "[-] CreatePipe (write) failed: %lu\n", GetLastError());
        CloseHandle(*phPipeIn);
        CloseHandle(*phPipePTYOut);
        return -1;
    }

    return 0;
}

/*
 * Create pseudo console with specified dimensions
 */
static int create_pseudo_console(ConPTY *pty, HANDLE hPipePTYIn,
                                  HANDLE hPipePTYOut, int cols, int rows)
{
    COORD size;
    HRESULT hr;

    size.X = (SHORT)cols;
    size.Y = (SHORT)rows;

    hr = CreatePseudoConsole(size, hPipePTYIn, hPipePTYOut, 0, &pty->hPC);
    if (FAILED(hr)) {
        fprintf(stderr, "[-] CreatePseudoConsole failed: 0x%lx\n", hr);
        return -1;
    }

    return 0;
}

/*
 * Spawn child process attached to the pseudo console
 */
static int spawn_process(ConPTY *pty, const char *cmdline)
{
    STARTUPINFOEXW si;
    PROCESS_INFORMATION pi;
    SIZE_T attrListSize = 0;
    LPPROC_THREAD_ATTRIBUTE_LIST attrList = NULL;
    wchar_t wcmdline[MAX_PATH];
    BOOL success = FALSE;

    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(STARTUPINFOEXW);
    ZeroMemory(&pi, sizeof(pi));

    /* Convert command line to wide string */
    MultiByteToWideChar(CP_UTF8, 0, cmdline, -1, wcmdline, MAX_PATH);

    /* Get required size for attribute list */
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);
    attrList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
        GetProcessHeap(), 0, attrListSize);
    if (!attrList) {
        fprintf(stderr, "[-] HeapAlloc failed\n");
        return -1;
    }

    /* Initialize attribute list */
    if (!InitializeProcThreadAttributeList(attrList, 1, 0, &attrListSize)) {
        fprintf(stderr, "[-] InitializeProcThreadAttributeList failed: %lu\n",
                GetLastError());
        HeapFree(GetProcessHeap(), 0, attrList);
        return -1;
    }

    /* Set pseudo console attribute */
    if (!UpdateProcThreadAttribute(attrList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                   pty->hPC, sizeof(HPCON), NULL, NULL)) {
        fprintf(stderr, "[-] UpdateProcThreadAttribute failed: %lu\n",
                GetLastError());
        DeleteProcThreadAttributeList(attrList);
        HeapFree(GetProcessHeap(), 0, attrList);
        return -1;
    }

    si.lpAttributeList = attrList;

    /* Create the process */
    success = CreateProcessW(
        NULL,                           /* lpApplicationName */
        wcmdline,                       /* lpCommandLine */
        NULL,                           /* lpProcessAttributes */
        NULL,                           /* lpThreadAttributes */
        FALSE,                          /* bInheritHandles */
        EXTENDED_STARTUPINFO_PRESENT,   /* dwCreationFlags */
        NULL,                           /* lpEnvironment */
        NULL,                           /* lpCurrentDirectory */
        &si.StartupInfo,                /* lpStartupInfo */
        &pi                             /* lpProcessInformation */
    );

    DeleteProcThreadAttributeList(attrList);
    HeapFree(GetProcessHeap(), 0, attrList);

    if (!success) {
        fprintf(stderr, "[-] CreateProcessW failed: %lu\n", GetLastError());
        return -1;
    }

    pty->hProcess = pi.hProcess;
    pty->hThread = pi.hThread;
    pty->dwProcessId = pi.dwProcessId;

    return 0;
}

/*
 * Read from PTY (non-blocking attempt)
 */
static int pty_read(ConPTY *pty, char *buf, size_t buflen, DWORD *bytesRead)
{
    DWORD available = 0;

    /* Check if data available (non-blocking) */
    if (!PeekNamedPipe(pty->hPipeIn, NULL, 0, NULL, &available, NULL)) {
        /* Pipe might be closed */
        return -1;
    }

    if (available == 0) {
        *bytesRead = 0;
        return 0;
    }

    /* Read available data */
    if (!ReadFile(pty->hPipeIn, buf, (DWORD)buflen - 1, bytesRead, NULL)) {
        fprintf(stderr, "[-] ReadFile failed: %lu\n", GetLastError());
        return -1;
    }

    buf[*bytesRead] = '\0';
    return 0;
}

/*
 * Write to PTY
 */
static int pty_write(ConPTY *pty, const char *buf, size_t len)
{
    DWORD written;

    if (!WriteFile(pty->hPipeOut, buf, (DWORD)len, &written, NULL)) {
        fprintf(stderr, "[-] WriteFile failed: %lu\n", GetLastError());
        return -1;
    }

    return (int)written;
}

/*
 * Resize pseudo console
 */
static int pty_resize(ConPTY *pty, int cols, int rows)
{
    COORD size;
    HRESULT hr;

    size.X = (SHORT)cols;
    size.Y = (SHORT)rows;

    hr = ResizePseudoConsole(pty->hPC, size);
    if (FAILED(hr)) {
        fprintf(stderr, "[-] ResizePseudoConsole failed: 0x%lx\n", hr);
        return -1;
    }

    return 0;
}

/*
 * Cleanup all resources
 */
static void pty_cleanup(ConPTY *pty)
{
    if (pty->hPC) {
        ClosePseudoConsole(pty->hPC);
        pty->hPC = NULL;
    }
    if (pty->hProcess) {
        TerminateProcess(pty->hProcess, 0);
        CloseHandle(pty->hProcess);
        pty->hProcess = NULL;
    }
    if (pty->hThread) {
        CloseHandle(pty->hThread);
        pty->hThread = NULL;
    }
    if (pty->hPipeIn) {
        CloseHandle(pty->hPipeIn);
        pty->hPipeIn = NULL;
    }
    if (pty->hPipeOut) {
        CloseHandle(pty->hPipeOut);
        pty->hPipeOut = NULL;
    }
}

/*
 * Read and print output with timeout
 */
static void read_and_print(ConPTY *pty, int timeout_ms)
{
    char buf[4096];
    DWORD bytesRead;
    int elapsed = 0;
    int interval = 100;

    while (elapsed < timeout_ms) {
        if (pty_read(pty, buf, sizeof(buf), &bytesRead) < 0) {
            break;
        }
        if (bytesRead > 0) {
            printf("%s", buf);
            fflush(stdout);
            elapsed = 0;  /* Reset timeout on activity */
        } else {
            Sleep(interval);
            elapsed += interval;
        }
    }
}

int main(int argc, char *argv[])
{
    ConPTY pty = {0};
    HANDLE hPipePTYIn = NULL, hPipePTYOut = NULL;
    int cols = 80, rows = 24;
    const char *shell = "cmd.exe";

    printf("[+] ConPTY POC - tmux Windows Port\n");
    printf("[+] Target: Windows 10 1809+ (build 17763+)\n\n");

    /* Allow custom shell */
    if (argc > 1) {
        shell = argv[1];
    }

    /* Step 1: Create pipes */
    printf("[+] Creating pipes...\n");
    if (create_pipes(&pty.hPipeIn, &pty.hPipeOut, &hPipePTYIn, &hPipePTYOut) < 0) {
        return 1;
    }

    /* Step 2: Create pseudo console */
    printf("[+] Creating pseudo console (%dx%d)...\n", cols, rows);
    if (create_pseudo_console(&pty, hPipePTYIn, hPipePTYOut, cols, rows) < 0) {
        CloseHandle(pty.hPipeIn);
        CloseHandle(pty.hPipeOut);
        CloseHandle(hPipePTYIn);
        CloseHandle(hPipePTYOut);
        return 1;
    }

    /* Close PTY-side pipe handles (now owned by pseudo console) */
    CloseHandle(hPipePTYIn);
    CloseHandle(hPipePTYOut);

    /* Step 3: Spawn process */
    printf("[+] Starting %s...\n", shell);
    if (spawn_process(&pty, shell) < 0) {
        pty_cleanup(&pty);
        return 1;
    }
    printf("[+] Process started, PID: %lu\n", pty.dwProcessId);

    /* Step 4: Read initial output (prompt) */
    printf("[+] Reading initial output...\n");
    printf("--- PTY OUTPUT START ---\n");
    read_and_print(&pty, 2000);
    printf("\n--- PTY OUTPUT END ---\n");

    /* Step 5: Send a command */
    printf("\n[+] Sending command: echo hello\n");
    pty_write(&pty, "echo hello\r\n", 12);

    /* Read response */
    printf("[+] Reading response...\n");
    printf("--- PTY OUTPUT START ---\n");
    read_and_print(&pty, 2000);
    printf("\n--- PTY OUTPUT END ---\n");

    /* Step 6: Test resize */
    printf("\n[+] Resizing to 120x40...\n");
    if (pty_resize(&pty, 120, 40) == 0) {
        printf("[+] Resize successful\n");
    } else {
        printf("[-] Resize failed\n");
    }

    /* Step 7: Send exit command */
    printf("\n[+] Sending exit command...\n");
    pty_write(&pty, "exit\r\n", 6);

    /* Wait a bit for process to exit */
    Sleep(500);

    /* Step 8: Cleanup */
    printf("[+] Cleanup...\n");
    pty_cleanup(&pty);

    printf("[+] Done!\n");
    return 0;
}
