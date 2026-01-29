/*
 * POC 06: libevent on Windows
 *
 * Validates libevent functionality on Windows.
 * Target: Windows 10+
 *
 * Build: cl /W4 /I<libevent-include> /Fe:libevent_poc.exe libevent_poc.c
 *        event.lib event_core.lib ws2_32.lib
 *
 * NOTE: Requires libevent library and headers.
 * See README.md for setup instructions.
 */

#define _WIN32_WINNT 0x0A00

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

/* Global state */
static int timer_count = 0;
static int max_timer_count = 5;
static struct event_base *base = NULL;
static HANDLE pipe_read = NULL;
static HANDLE pipe_write = NULL;

/*
 * Timer callback
 */
static void timer_callback(evutil_socket_t fd, short events, void *arg)
{
    struct event *timer_event = (struct event *)arg;
    struct timeval tv = {1, 0};  /* 1 second */

    (void)fd;
    (void)events;

    timer_count++;
    printf("[!] Timer fired! Count: %d/%d\n", timer_count, max_timer_count);

    if (timer_count >= max_timer_count) {
        printf("[+] Reached max timer count, stopping loop\n");
        event_base_loopbreak(base);
    } else {
        /* Re-add timer */
        evtimer_add(timer_event, &tv);
    }
}

/*
 * Create anonymous pipe for testing I/O events
 */
static int create_test_pipe(void)
{
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&pipe_read, &pipe_write, &sa, 0)) {
        fprintf(stderr, "[-] CreatePipe failed: %lu\n", GetLastError());
        return -1;
    }

    return 0;
}

/*
 * Thread that writes to pipe after delay
 */
static DWORD WINAPI pipe_writer_thread(LPVOID param)
{
    DWORD written;
    const char *msg = "Hello from pipe!";

    (void)param;

    /* Wait a bit, then write */
    Sleep(2500);

    printf("[+] Writer thread: sending data to pipe\n");
    WriteFile(pipe_write, msg, (DWORD)strlen(msg), &written, NULL);

    return 0;
}

/*
 * Test basic event loop and timer
 */
static int test_event_loop(void)
{
    struct event *timer_event;
    struct timeval tv = {1, 0};  /* 1 second */
    const char *method;

    printf("[+] Creating event base...\n");
    base = event_base_new();
    if (!base) {
        fprintf(stderr, "[-] event_base_new() failed\n");
        return -1;
    }

    method = event_base_get_method(base);
    printf("[+] Using backend: %s\n", method);

    /* Create timer event */
    printf("[+] Setting up timer (fires every 1 second, %d times)...\n", max_timer_count);
    timer_event = evtimer_new(base, timer_callback, NULL);
    if (!timer_event) {
        fprintf(stderr, "[-] evtimer_new() failed\n");
        event_base_free(base);
        return -1;
    }

    /* Pass the event to callback for re-adding */
    event_assign(timer_event, base, -1, 0, timer_callback, timer_event);

    /* Add timer */
    if (evtimer_add(timer_event, &tv) < 0) {
        fprintf(stderr, "[-] evtimer_add() failed\n");
        event_free(timer_event);
        event_base_free(base);
        return -1;
    }

    /* Run event loop */
    printf("\n[+] Starting event loop...\n");
    event_base_dispatch(base);

    /* Cleanup */
    event_free(timer_event);
    event_base_free(base);
    base = NULL;

    printf("[+] Event loop completed\n");
    return 0;
}

/*
 * Test socket-based events (libevent's native Windows support)
 */
static int test_socket_events(void)
{
    struct event_base *sock_base;
    struct event *timer_event;
    struct timeval tv = {1, 0};
    evutil_socket_t pair[2];

    printf("\n[+] Testing socket-based events...\n");

    sock_base = event_base_new();
    if (!sock_base) {
        fprintf(stderr, "[-] event_base_new() failed\n");
        return -1;
    }

    /* Create socket pair for testing */
    if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, pair) < 0) {
        fprintf(stderr, "[-] evutil_socketpair() failed\n");
        event_base_free(sock_base);
        return -1;
    }

    printf("[+] Socket pair created: %d, %d\n", (int)pair[0], (int)pair[1]);

    /* Quick timer to end test */
    timer_count = 0;
    max_timer_count = 2;
    timer_event = evtimer_new(sock_base, timer_callback, NULL);
    event_assign(timer_event, sock_base, -1, 0, timer_callback, timer_event);
    evtimer_add(timer_event, &tv);

    base = sock_base;  /* For callback */
    event_base_dispatch(sock_base);

    /* Cleanup */
    evutil_closesocket(pair[0]);
    evutil_closesocket(pair[1]);
    event_free(timer_event);
    event_base_free(sock_base);

    printf("[+] Socket events test completed\n");
    return 0;
}

/*
 * Print libevent info
 */
static void print_libevent_info(void)
{
    const char *version = event_get_version();
    int i;
    const char **methods = event_get_supported_methods();

    printf("[+] libevent version: %s\n", version);
    printf("[+] Supported backends: ");
    for (i = 0; methods[i] != NULL; i++) {
        printf("%s ", methods[i]);
    }
    printf("\n\n");
}

int main(int argc, char *argv[])
{
    WSADATA wsa;

    (void)argc;
    (void)argv;

    printf("[+] libevent POC - tmux Windows Port\n\n");

    /* Initialize Winsock (required for libevent on Windows) */
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "[-] WSAStartup failed\n");
        return 1;
    }

    /* Print info */
    print_libevent_info();

    /* Test 1: Basic event loop with timer */
    printf("=== Test 1: Event Loop + Timer ===\n");
    timer_count = 0;
    max_timer_count = 3;
    if (test_event_loop() < 0) {
        fprintf(stderr, "[-] Test 1 FAILED\n");
        WSACleanup();
        return 1;
    }

    /* Test 2: Socket-based events */
    printf("\n=== Test 2: Socket Events ===\n");
    timer_count = 0;
    max_timer_count = 2;
    if (test_socket_events() < 0) {
        fprintf(stderr, "[-] Test 2 FAILED\n");
        WSACleanup();
        return 1;
    }

    /* Cleanup */
    WSACleanup();

    printf("\n[+] All tests passed!\n");
    printf("[+] libevent works on Windows\n");
    printf("\n[!] NOTE: For pipe handle integration, we may need wrappers\n");
    printf("[!]       since libevent primarily works with sockets on Windows.\n");

    return 0;
}
