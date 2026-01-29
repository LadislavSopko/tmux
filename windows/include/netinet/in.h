/*
 * netinet/in.h - Windows compatibility shim
 *
 * Provides network byte order functions and IP address structures.
 * Most of this is already in winsock2.h, we just include it.
 */

#ifndef COMPAT_NETINET_IN_H
#define COMPAT_NETINET_IN_H

#ifdef _WIN32

/* winsock2.h provides htonl, htons, ntohl, ntohs, in_addr, sockaddr_in */
#include <winsock2.h>
#include <ws2tcpip.h>

/* IPPROTO constants - should be in winsock2.h but define if missing */
#ifndef IPPROTO_TCP
#define IPPROTO_TCP 6
#endif
#ifndef IPPROTO_UDP
#define IPPROTO_UDP 17
#endif

/* INADDR constants */
#ifndef INADDR_ANY
#define INADDR_ANY       ((unsigned long)0x00000000)
#endif
#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK  ((unsigned long)0x7f000001)
#endif
#ifndef INADDR_NONE
#define INADDR_NONE      ((unsigned long)0xffffffff)
#endif

#endif /* _WIN32 */
#endif /* COMPAT_NETINET_IN_H */
