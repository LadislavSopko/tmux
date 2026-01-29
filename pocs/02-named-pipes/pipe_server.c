/*
 * POC 02: Named Pipes Server
 *
 * Validates Named Pipes as replacement for Unix Domain Sockets.
 * Target: Windows 10+
 *
 * Build: cl /W4 /Fe:pipe_server.exe pipe_server.c
 */

#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>

#define PIPE_NAME "\\\\.\\pipe\\tmux-test"
#define BUFFER_SIZE 4096

/*
 * Handle a single client connection
 */
static int handle_client(HANDLE hPipe)
{
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;
    const char *response = "Hello from server";

    printf("[+] Client connected!\n");

    /* Read message from client */
    if (!ReadFile(hPipe, buffer, BUFFER_SIZE - 1, &bytesRead, NULL)) {
        fprintf(stderr, "[-] ReadFile failed: %lu\n", GetLastError());
        return -1;
    }
    buffer[bytesRead] = '\0';
    printf("[+] Received: %s\n", buffer);

    /* Send response */
    if (!WriteFile(hPipe, response, (DWORD)strlen(response), &bytesWritten, NULL)) {
        fprintf(stderr, "[-] WriteFile failed: %lu\n", GetLastError());
        return -1;
    }
    printf("[+] Sent response: %s\n", response);

    /* Flush and disconnect */
    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    printf("[+] Client disconnected\n");

    return 0;
}

/*
 * Create named pipe server instance
 */
static HANDLE create_pipe_server(void)
{
    HANDLE hPipe;

    hPipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        BUFFER_SIZE,
        BUFFER_SIZE,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[-] CreateNamedPipe failed: %lu\n", GetLastError());
        return NULL;
    }

    return hPipe;
}

/*
 * Create additional pipe instance for next client
 */
static HANDLE create_pipe_instance(void)
{
    HANDLE hPipe;

    hPipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        BUFFER_SIZE,
        BUFFER_SIZE,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[-] CreateNamedPipe (instance) failed: %lu\n", GetLastError());
        return NULL;
    }

    return hPipe;
}

int main(int argc, char *argv[])
{
    HANDLE hPipe;
    int client_count = 0;
    int max_clients = 3;  /* Handle 3 clients then exit */

    (void)argc;
    (void)argv;

    printf("[+] Named Pipes Server POC - tmux Windows Port\n");
    printf("[+] Creating pipe: %s\n", PIPE_NAME);

    /* Create first pipe instance */
    hPipe = create_pipe_server();
    if (!hPipe) {
        return 1;
    }

    printf("[+] Server ready. Will handle %d clients then exit.\n", max_clients);
    printf("[+] Run pipe_client.exe in another terminal.\n\n");

    while (client_count < max_clients) {
        printf("[+] Waiting for client %d/%d...\n", client_count + 1, max_clients);

        /* Wait for client connection */
        if (!ConnectNamedPipe(hPipe, NULL)) {
            DWORD err = GetLastError();
            if (err != ERROR_PIPE_CONNECTED) {
                fprintf(stderr, "[-] ConnectNamedPipe failed: %lu\n", err);
                CloseHandle(hPipe);
                return 1;
            }
        }

        /* Handle client */
        handle_client(hPipe);
        client_count++;

        /* Prepare for next client if not done */
        if (client_count < max_clients) {
            /* Reuse same pipe handle, just reconnect */
            /* DisconnectNamedPipe was already called in handle_client */
        }
    }

    printf("\n[+] Handled %d clients. Server exiting.\n", max_clients);
    CloseHandle(hPipe);

    return 0;
}
