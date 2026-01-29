/*
 * arpa/inet.h - Windows compatibility shim
 *
 * Provides network byte order functions and IP address conversion.
 */

#ifndef COMPAT_ARPA_INET_H
#define COMPAT_ARPA_INET_H

#ifdef _WIN32

/* winsock2.h provides htonl, htons, ntohl, ntohs, inet_addr, inet_ntoa */
#include <winsock2.h>
#include <ws2tcpip.h>

/* inet_pton and inet_ntop are in ws2tcpip.h on Windows Vista+ */

#endif /* _WIN32 */
#endif /* COMPAT_ARPA_INET_H */
