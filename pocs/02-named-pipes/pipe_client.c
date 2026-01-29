/*
 * POC 02: Named Pipes Client
 *
 * Validates Named Pipes as replacement for Unix Domain Sockets.
 * Target: Windows 10+
 *
 * Build: cl /W4 /Fe:pipe_client.exe pipe_client.c
 */

#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>

#define PIPE_NAME "\\\\.\\pipe\\tmux-test"
#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    HANDLE hPipe;
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;
    const char *message = "Hello from client";
    DWORD mode = PIPE_READMODE_MESSAGE;

    (void)argc;
    (void)argv;

    printf("[+] Named Pipes Client POC - tmux Windows Port\n");
    printf("[+] Connecting to: %s\n", PIPE_NAME);

    /* Try to connect to pipe */
    hPipe = CreateFileA(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            fprintf(stderr, "[-] Pipe not found. Is the server running?\n");
        } else if (err == ERROR_PIPE_BUSY) {
            fprintf(stderr, "[-] Pipe busy. Server is handling another client.\n");
            fprintf(stderr, "    Try again in a moment.\n");
        } else {
            fprintf(stderr, "[-] CreateFile failed: %lu\n", err);
        }
        return 1;
    }

    printf("[+] Connected!\n");

    /* Set pipe to message mode */
    if (!SetNamedPipeHandleState(hPipe, &mode, NULL, NULL)) {
        fprintf(stderr, "[-] SetNamedPipeHandleState failed: %lu\n", GetLastError());
        CloseHandle(hPipe);
        return 1;
    }

    /* Send message */
    printf("[+] Sending: %s\n", message);
    if (!WriteFile(hPipe, message, (DWORD)strlen(message), &bytesWritten, NULL)) {
        fprintf(stderr, "[-] WriteFile failed: %lu\n", GetLastError());
        CloseHandle(hPipe);
        return 1;
    }

    /* Read response */
    if (!ReadFile(hPipe, buffer, BUFFER_SIZE - 1, &bytesRead, NULL)) {
        fprintf(stderr, "[-] ReadFile failed: %lu\n", GetLastError());
        CloseHandle(hPipe);
        return 1;
    }
    buffer[bytesRead] = '\0';
    printf("[+] Received: %s\n", buffer);

    CloseHandle(hPipe);
    printf("[+] Done!\n");

    return 0;
}
