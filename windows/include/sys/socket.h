/*
 * sys/socket.h - Windows compatibility shim
 * Note: Include AFTER winsock2.h to avoid conflicts
 */

#ifndef _SYS_SOCKET_H_WIN32
#define _SYS_SOCKET_H_WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

/* Socket constants if not defined */
#ifndef AF_UNIX
#define AF_UNIX 1
#endif

#ifndef SOCK_STREAM
#define SOCK_STREAM 1
#endif

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0x40
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef SO_PEERCRED
#define SO_PEERCRED 17
#endif

#ifndef SCM_RIGHTS
#define SCM_RIGHTS 1
#endif

/* Credential structure (stub for Unix compatibility) */
struct ucred {
    int pid;
    int uid;
    int gid;
};

#endif /* _SYS_SOCKET_H_WIN32 */
