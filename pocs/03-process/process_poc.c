/*
 * POC 03: Process Management
 *
 * Validates CreateProcess as replacement for fork()+exec().
 * Target: Windows 10+
 *
 * Build: cl /W4 /Fe:process_poc.exe process_poc.c
 */

#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>

/*
 * Spawn a process and return handles
 */
static int spawn_process(const char *cmdline, const char *cwd,
                         char *env, HANDLE *phProcess, DWORD *pdwPid)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char cmdline_copy[MAX_PATH];

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    /* CreateProcess may modify cmdline, so copy it */
    strncpy(cmdline_copy, cmdline, MAX_PATH - 1);
    cmdline_copy[MAX_PATH - 1] = '\0';

    if (!CreateProcessA(
            NULL,           /* lpApplicationName */
            cmdline_copy,   /* lpCommandLine */
            NULL,           /* lpProcessAttributes */
            NULL,           /* lpThreadAttributes */
            FALSE,          /* bInheritHandles */
            0,              /* dwCreationFlags */
            env,            /* lpEnvironment (NULL = inherit) */
            cwd,            /* lpCurrentDirectory */
            &si,            /* lpStartupInfo */
            &pi             /* lpProcessInformation */
        )) {
        fprintf(stderr, "[-] CreateProcess failed: %lu\n", GetLastError());
        return -1;
    }

    *phProcess = pi.hProcess;
    *pdwPid = pi.dwProcessId;

    /* Close thread handle, we don't need it */
    CloseHandle(pi.hThread);

    return 0;
}

/*
 * Wait for process to exit (blocking)
 */
static int wait_process(HANDLE hProcess, DWORD *pExitCode)
{
    DWORD result;

    result = WaitForSingleObject(hProcess, INFINITE);
    if (result != WAIT_OBJECT_0) {
        fprintf(stderr, "[-] WaitForSingleObject failed: %lu\n", GetLastError());
        return -1;
    }

    if (!GetExitCodeProcess(hProcess, pExitCode)) {
        fprintf(stderr, "[-] GetExitCodeProcess failed: %lu\n", GetLastError());
        return -1;
    }

    return 0;
}

/*
 * Check if process has exited (non-blocking)
 */
static int poll_process(HANDLE hProcess, int *pDone, DWORD *pExitCode)
{
    DWORD result;

    result = WaitForSingleObject(hProcess, 0);  /* 0 timeout = poll */

    if (result == WAIT_OBJECT_0) {
        *pDone = 1;
        if (!GetExitCodeProcess(hProcess, pExitCode)) {
            fprintf(stderr, "[-] GetExitCodeProcess failed: %lu\n", GetLastError());
            return -1;
        }
    } else if (result == WAIT_TIMEOUT) {
        *pDone = 0;
    } else {
        fprintf(stderr, "[-] WaitForSingleObject failed: %lu\n", GetLastError());
        return -1;
    }

    return 0;
}

/*
 * Terminate a process
 */
static int terminate_process(HANDLE hProcess)
{
    if (!TerminateProcess(hProcess, 1)) {
        fprintf(stderr, "[-] TerminateProcess failed: %lu\n", GetLastError());
        return -1;
    }
    return 0;
}

/*
 * Test 1: Basic spawn and wait
 */
static int test_spawn_and_wait(void)
{
    HANDLE hProcess;
    DWORD dwPid, exitCode;

    printf("[+] Test 1: Spawn and wait...\n");

    /* Run a simple command */
    if (spawn_process("cmd.exe /c echo test", NULL, NULL, &hProcess, &dwPid) < 0) {
        return -1;
    }
    printf("[+] Created process PID: %lu\n", dwPid);

    /* Wait for it */
    if (wait_process(hProcess, &exitCode) < 0) {
        CloseHandle(hProcess);
        return -1;
    }
    printf("[+] Process exited with code: %lu\n", exitCode);

    CloseHandle(hProcess);
    return 0;
}

/*
 * Test 2: Custom environment
 */
static int test_custom_environment(void)
{
    HANDLE hProcess;
    DWORD dwPid, exitCode;

    /* Environment block: VAR1=val1\0VAR2=val2\0\0 */
    char env[] = "MY_VAR=hello_from_poc\0PATH=C:\\Windows\\System32\0\0";

    printf("\n[+] Test 2: Custom environment...\n");

    /* Echo the custom variable */
    if (spawn_process("cmd.exe /c echo MY_VAR=%MY_VAR%", NULL, env, &hProcess, &dwPid) < 0) {
        return -1;
    }
    printf("[+] Created process with custom env, PID: %lu\n", dwPid);

    /* Wait for it */
    if (wait_process(hProcess, &exitCode) < 0) {
        CloseHandle(hProcess);
        return -1;
    }
    printf("[+] Process exited with code: %lu\n", exitCode);

    CloseHandle(hProcess);
    return 0;
}

/*
 * Test 3: Non-blocking wait (polling)
 */
static int test_polling(void)
{
    HANDLE hProcess;
    DWORD dwPid, exitCode;
    int done;
    int polls = 0;

    printf("\n[+] Test 3: Non-blocking wait (polling)...\n");

    /* Run a command that takes a bit of time */
    if (spawn_process("cmd.exe /c ping -n 2 127.0.0.1 >nul", NULL, NULL, &hProcess, &dwPid) < 0) {
        return -1;
    }
    printf("[+] Created process PID: %lu\n", dwPid);
    printf("[+] Polling...\n");

    /* Poll until done */
    do {
        if (poll_process(hProcess, &done, &exitCode) < 0) {
            CloseHandle(hProcess);
            return -1;
        }
        if (!done) {
            printf("    Poll %d: still running...\n", ++polls);
            Sleep(500);
        }
    } while (!done);

    printf("[+] Process done after %d polls, exit code: %lu\n", polls, exitCode);

    CloseHandle(hProcess);
    return 0;
}

/*
 * Test 4: Terminate a running process
 */
static int test_terminate(void)
{
    HANDLE hProcess;
    DWORD dwPid, exitCode;
    int done;

    printf("\n[+] Test 4: Terminate process...\n");

    /* Start a long-running process */
    if (spawn_process("cmd.exe /c ping -n 100 127.0.0.1 >nul", NULL, NULL, &hProcess, &dwPid) < 0) {
        return -1;
    }
    printf("[+] Started long-running process PID: %lu\n", dwPid);

    /* Let it run a bit */
    Sleep(500);

    /* Check it's still running */
    poll_process(hProcess, &done, &exitCode);
    if (done) {
        printf("[-] Process already exited (unexpected)\n");
        CloseHandle(hProcess);
        return -1;
    }
    printf("[+] Process is running\n");

    /* Terminate it */
    printf("[+] Terminating...\n");
    if (terminate_process(hProcess) < 0) {
        CloseHandle(hProcess);
        return -1;
    }

    /* Wait for termination */
    wait_process(hProcess, &exitCode);
    printf("[+] Process terminated, exit code: %lu\n", exitCode);

    CloseHandle(hProcess);
    return 0;
}

/*
 * Test 5: Working directory
 */
static int test_working_directory(void)
{
    HANDLE hProcess;
    DWORD dwPid, exitCode;

    printf("\n[+] Test 5: Working directory...\n");

    /* Run in C:\Windows */
    if (spawn_process("cmd.exe /c cd", "C:\\Windows", NULL, &hProcess, &dwPid) < 0) {
        return -1;
    }
    printf("[+] Created process with cwd=C:\\Windows, PID: %lu\n", dwPid);

    /* Wait for it */
    if (wait_process(hProcess, &exitCode) < 0) {
        CloseHandle(hProcess);
        return -1;
    }
    printf("[+] Process exited with code: %lu (output should show C:\\Windows)\n", exitCode);

    CloseHandle(hProcess);
    return 0;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("[+] Process Management POC - tmux Windows Port\n\n");

    if (test_spawn_and_wait() < 0) {
        printf("[-] Test 1 FAILED\n");
        return 1;
    }

    if (test_custom_environment() < 0) {
        printf("[-] Test 2 FAILED\n");
        return 1;
    }

    if (test_polling() < 0) {
        printf("[-] Test 3 FAILED\n");
        return 1;
    }

    if (test_terminate() < 0) {
        printf("[-] Test 4 FAILED\n");
        return 1;
    }

    if (test_working_directory() < 0) {
        printf("[-] Test 5 FAILED\n");
        return 1;
    }

    printf("\n[+] All tests passed!\n");
    return 0;
}
