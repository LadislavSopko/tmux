/*
 * sys/utsname.h - Windows compatibility shim
 *
 * Provides uname() function and utsname structure.
 */

#ifndef COMPAT_SYS_UTSNAME_H
#define COMPAT_SYS_UTSNAME_H

#ifdef _WIN32

#include <windows.h>
#include <string.h>

#define _UTSNAME_LENGTH 256

struct utsname {
    char sysname[_UTSNAME_LENGTH];   /* OS name */
    char nodename[_UTSNAME_LENGTH];  /* Network node name */
    char release[_UTSNAME_LENGTH];   /* OS release */
    char version[_UTSNAME_LENGTH];   /* OS version */
    char machine[_UTSNAME_LENGTH];   /* Hardware type */
};

static __inline int uname(struct utsname *name)
{
    OSVERSIONINFOEXW osvi;
    SYSTEM_INFO si;
    DWORD size;

    if (name == NULL)
        return -1;

    memset(name, 0, sizeof(*name));

    /* System name */
    strcpy(name->sysname, "Windows");

    /* Node name (computer name) */
    size = _UTSNAME_LENGTH;
    if (!GetComputerNameA(name->nodename, &size))
        strcpy(name->nodename, "unknown");

    /* Version info */
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    /* Use RtlGetVersion for accurate version on Windows 10+ */
    typedef LONG (WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(
        GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");

    if (RtlGetVersion) {
        RtlGetVersion((PRTL_OSVERSIONINFOW)&osvi);
    }

    sprintf(name->release, "%lu.%lu", osvi.dwMajorVersion, osvi.dwMinorVersion);
    sprintf(name->version, "Build %lu", osvi.dwBuildNumber);

    /* Machine type */
    GetNativeSystemInfo(&si);
    switch (si.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        strcpy(name->machine, "x86_64");
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        strcpy(name->machine, "aarch64");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        strcpy(name->machine, "i686");
        break;
    default:
        strcpy(name->machine, "unknown");
        break;
    }

    return 0;
}

#endif /* _WIN32 */
#endif /* COMPAT_SYS_UTSNAME_H */
