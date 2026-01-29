/*
 * imsg-win32.c - Message protocol stubs for Windows
 *
 * These are minimal stubs to allow linking. Full implementation
 * will use Named Pipes in Phase 3 (IPC Layer).
 */

#include "compat-win32.h"
#include "tmux.h"

/* Structures defined in compat/imsg.h */

/*
 * imsgbuf_init - initialize message buffer
 */
int
imsgbuf_init(struct imsgbuf *imsgbuf, int fd)
{
    if (imsgbuf == NULL)
        return -1;
    imsgbuf->fd = fd;
    imsgbuf->flags = 0;
    return 0;
}

/*
 * imsgbuf_allow_fdpass - allow file descriptor passing
 */
void
imsgbuf_allow_fdpass(struct imsgbuf *imsgbuf)
{
    (void)imsgbuf;
    /* No-op on Windows - fd passing not supported via named pipes */
}

/*
 * imsgbuf_read - read messages from buffer
 */
int
imsgbuf_read(struct imsgbuf *imsgbuf)
{
    (void)imsgbuf;
    /* TODO: Implement with Named Pipes */
    return 0;
}

/*
 * imsgbuf_write - write messages to buffer
 */
int
imsgbuf_write(struct imsgbuf *imsgbuf)
{
    (void)imsgbuf;
    /* TODO: Implement with Named Pipes */
    return 0;
}

/*
 * imsgbuf_flush - flush message buffer
 */
int
imsgbuf_flush(struct imsgbuf *imsgbuf)
{
    (void)imsgbuf;
    /* TODO: Implement */
    return 0;
}

/*
 * imsgbuf_clear - clear message buffer
 */
void
imsgbuf_clear(struct imsgbuf *imsgbuf)
{
    (void)imsgbuf;
    /* TODO: Implement */
}

/*
 * imsgbuf_queuelen - get queue length
 */
uint32_t
imsgbuf_queuelen(struct imsgbuf *imsgbuf)
{
    (void)imsgbuf;
    return 0;
}

/*
 * imsg_get - get next message from buffer
 */
ssize_t
imsg_get(struct imsgbuf *imsgbuf, struct imsg *imsg)
{
    (void)imsgbuf;
    (void)imsg;
    /* TODO: Implement */
    return 0;  /* No message available */
}

/*
 * imsg_compose - compose and queue a message
 */
int
imsg_compose(struct imsgbuf *imsgbuf, uint32_t type, uint32_t peerid,
    pid_t pid, int fd, const void *data, size_t datalen)
{
    (void)imsgbuf;
    (void)type;
    (void)peerid;
    (void)pid;
    (void)fd;
    (void)data;
    (void)datalen;
    /* TODO: Implement */
    return 0;
}

/*
 * imsg_free - free message data
 */
void
imsg_free(struct imsg *imsg)
{
    (void)imsg;
    /* TODO: Implement */
}

/*
 * imsg_get_fd - get file descriptor from message
 */
int
imsg_get_fd(struct imsg *imsg)
{
    (void)imsg;
    return -1;  /* No fd passing on Windows */
}
