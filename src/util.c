#include "util.h"

int endofhdr(const char *msgbuf, const size_t len)
{
    return len >= 2 && msgbuf[len - 2] == '\r' && msgbuf[len - 1] == '\n';
}

int endofmsg(const char *msgbuf, const size_t len)
{
    return len == 2 && msgbuf[0] == '\r' && msgbuf[1] == '\n';
}

ssize_t read_line(int fd, char *buffer, size_t size)
{
    size_t n = 0;
    ssize_t nread = 0;

    for (; n < size;)
    {
        nread = read(fd, (void *)(buffer + n), 1);

        if (nread == -1)
            return ERROR;

        n += (size_t)nread;

        if (endofhdr(buffer, n))
            break;
    }

    buffer[n] = '\0';

    return n;
}
