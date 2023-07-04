#include "http.h"
#include "util.h"

int endofhdr(const char *msgbuf, const size_t len)
{
    return len >= 2 && msgbuf[len - 2] == '\r' && msgbuf[len - 1] == '\n';
}

int endofmsg(const char *msgbuf, const size_t len)
{
    return len == 2 && msgbuf[0] == '\r' && msgbuf[1] == '\n';
}

/* Read one line sent by `fd` socket and store into `msgbuf`.

On success, it returns the number of read bytes if there is any content. On
error, it will returns -1.
*/
ssize_t readline(int fd, char *msgbuf)
{
    size_t n = 0;
    ssize_t nread = 0;

    for (;;)
    {
        nread = recv(fd, (void *)(msgbuf + n), 1, 0);

        if (nread == -1)
            return -1;

        n += (size_t)nread;

        if (endofhdr(msgbuf, n))
            break;
    }

    msgbuf[n] = '\0';

    return n;
}

ssize_t readfrom(int fd)
{
    ssize_t nread = 0;
    size_t n = 0;
    char buffer[MSGSIZE];
    memset(buffer, '\0', MSGSIZE);

    for (;;)
    {
        nread = readline(fd, buffer);

        if (nread == -1)
            return -1;

        lower(buffer);
        write(STDOUT_FILENO, (const void *)buffer, (size_t)nread);
        n += (size_t)nread;

        if (endofmsg(buffer, (size_t)nread))
            break;
    }

    dprintf(stdout, "End of message\n");
    dprintf(stdout, "Byte read: %ld\n", n);
    dprintf(stdout, "==================\n\n");
    return n;
}

int handle(int fd)
{
    ssize_t nread;
    int status = 0;

    nread = readfrom(fd);

    if (nread == -1)
    {
        perror("recv");
        status = (int)nread;
        goto safe_exit;
    }

safe_exit:
    close(fd);
    return status;
}
