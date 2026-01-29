/*
 * pwd.h - Windows compatibility shim
 */

#ifndef _PWD_H_WIN32
#define _PWD_H_WIN32

#include <stdlib.h>
#include <string.h>

struct passwd {
    char *pw_name;
    char *pw_passwd;
    int   pw_uid;
    int   pw_gid;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};

/* Static storage for passwd struct */
static struct passwd _win32_passwd;
static char _win32_username[256];
static char _win32_homedir[MAX_PATH];

static __inline struct passwd *getpwuid(int uid)
{
    DWORD size;
    (void)uid;

    size = sizeof(_win32_username);
    if (!GetUserNameA(_win32_username, &size)) {
        strcpy(_win32_username, "user");
    }

    if (!GetEnvironmentVariableA("USERPROFILE", _win32_homedir, sizeof(_win32_homedir))) {
        strcpy(_win32_homedir, "C:\\Users\\Default");
    }

    _win32_passwd.pw_name = _win32_username;
    _win32_passwd.pw_passwd = "";
    _win32_passwd.pw_uid = 0;
    _win32_passwd.pw_gid = 0;
    _win32_passwd.pw_gecos = _win32_username;
    _win32_passwd.pw_dir = _win32_homedir;
    _win32_passwd.pw_shell = "cmd.exe";

    return &_win32_passwd;
}

static __inline struct passwd *getpwnam(const char *name)
{
    (void)name;
    return getpwuid(0);
}

static __inline void endpwent(void) {}
static __inline void setpwent(void) {}
static __inline struct passwd *getpwent(void) { return NULL; }

#endif /* _PWD_H_WIN32 */
