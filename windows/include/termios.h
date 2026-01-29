/*
 * termios.h - Windows compatibility shim
 * Maps terminal I/O to Windows Console API
 */

#ifndef _TERMIOS_H_WIN32
#define _TERMIOS_H_WIN32

#include <windows.h>

/* Speed types */
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;
typedef unsigned char cc_t;

/* Control characters */
#define NCCS 32
#define VEOF     0
#define VEOL     1
#define VERASE   2
#define VINTR    3
#define VKILL    4
#define VMIN     5
#define VQUIT    6
#define VSTART   7
#define VSTOP    8
#define VSUSP    9
#define VTIME    10
#define VLNEXT   11
#define VWERASE  12

/* Input flags */
#define IGNBRK  0x00001
#define BRKINT  0x00002
#define IGNPAR  0x00004
#define PARMRK  0x00008
#define INPCK   0x00010
#define ISTRIP  0x00020
#define INLCR   0x00040
#define IGNCR   0x00080
#define ICRNL   0x00100
#define IXON    0x00200
#define IXOFF   0x00400
#define IXANY   0x00800
#define IMAXBEL 0x01000
#define IUTF8   0x02000

/* Output flags */
#define OPOST   0x00001
#define ONLCR   0x00002
#define OCRNL   0x00004
#define ONOCR   0x00008
#define ONLRET  0x00010

/* Control flags */
#define CSIZE   0x00030
#define CS5     0x00000
#define CS6     0x00010
#define CS7     0x00020
#define CS8     0x00030
#define CSTOPB  0x00040
#define CREAD   0x00080
#define PARENB  0x00100
#define PARODD  0x00200
#define HUPCL   0x00400
#define CLOCAL  0x00800

/* Local flags */
#define ISIG    0x00001
#define ICANON  0x00002
#define ECHO    0x00004
#define ECHOE   0x00008
#define ECHOK   0x00010
#define ECHONL  0x00020
#define NOFLSH  0x00040
#define TOSTOP  0x00080
#define IEXTEN  0x00100
#define ECHOCTL 0x00200
#define ECHOKE  0x00400

/* Baud rates */
#define B0      0
#define B50     50
#define B75     75
#define B110    110
#define B134    134
#define B150    150
#define B200    200
#define B300    300
#define B600    600
#define B1200   1200
#define B1800   1800
#define B2400   2400
#define B4800   4800
#define B9600   9600
#define B19200  19200
#define B38400  38400
#define B57600  57600
#define B115200 115200
#define B230400 230400

/* tcsetattr actions */
#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

/* tcflush queue selectors */
#define TCIFLUSH  0
#define TCOFLUSH  1
#define TCIOFLUSH 2

/* tcflow actions */
#define TCOOFF 0
#define TCOON  1
#define TCIOFF 2
#define TCION  3

/* termios structure */
struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t     c_cc[NCCS];
    speed_t  c_ispeed;
    speed_t  c_ospeed;
};

/* Stub implementations */
static __inline int tcgetattr(int fd, struct termios *termios_p)
{
    (void)fd;
    if (termios_p) {
        memset(termios_p, 0, sizeof(*termios_p));
        termios_p->c_iflag = ICRNL | IXON;
        termios_p->c_oflag = OPOST | ONLCR;
        termios_p->c_cflag = CS8 | CREAD;
        termios_p->c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK;
    }
    return 0;
}

static __inline int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
    (void)fd; (void)optional_actions; (void)termios_p;
    return 0;
}

static __inline speed_t cfgetispeed(const struct termios *termios_p)
{
    return termios_p ? termios_p->c_ispeed : B9600;
}

static __inline speed_t cfgetospeed(const struct termios *termios_p)
{
    return termios_p ? termios_p->c_ospeed : B9600;
}

static __inline int cfsetispeed(struct termios *termios_p, speed_t speed)
{
    if (termios_p) termios_p->c_ispeed = speed;
    return 0;
}

static __inline int cfsetospeed(struct termios *termios_p, speed_t speed)
{
    if (termios_p) termios_p->c_ospeed = speed;
    return 0;
}

static __inline void cfmakeraw(struct termios *termios_p)
{
    if (termios_p) {
        termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
        termios_p->c_oflag &= ~OPOST;
        termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        termios_p->c_cflag &= ~(CSIZE | PARENB);
        termios_p->c_cflag |= CS8;
    }
}

static __inline int tcflush(int fd, int queue_selector)
{
    (void)fd; (void)queue_selector;
    return 0;
}

static __inline int tcdrain(int fd)
{
    (void)fd;
    return 0;
}

static __inline int tcflow(int fd, int action)
{
    (void)fd; (void)action;
    return 0;
}

static __inline int tcsendbreak(int fd, int duration)
{
    (void)fd; (void)duration;
    return 0;
}

#endif /* _TERMIOS_H_WIN32 */
