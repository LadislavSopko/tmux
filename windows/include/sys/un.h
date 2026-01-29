/*
 * sys/un.h - Windows compatibility shim for Unix domain sockets
 */

#ifndef _SYS_UN_H_WIN32
#define _SYS_UN_H_WIN32

#ifndef AF_UNIX
#define AF_UNIX 1
#endif

/* sockaddr_un structure */
#ifndef _SOCKADDR_UN_DEFINED
#define _SOCKADDR_UN_DEFINED
struct sockaddr_un {
    unsigned short sun_family;
    char sun_path[108];
};
#endif

#endif /* _SYS_UN_H_WIN32 */
