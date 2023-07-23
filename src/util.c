#include "util.h"

ssize_t
read_until(int fd, void *buf, size_t buflen, const char *delim, int flags)
{
    const size_t delim_len = strlen(delim);

    ssize_t nread = 0;
    size_t total  = 0;
    size_t cmp_idx;

    for (; total < buflen;)
    {
        nread = recv(fd, (char *)(buf + total), 1, flags);

        if (nread == -1)
            return -1;
        else if (nread == 0)
            return 0;

        total += (size_t)nread;

        cmp_idx = total - delim_len;
        if (strncmp((char *)(buf + cmp_idx), delim, delim_len) == 0)
            break;
    }

    // Only read the content before the delimiter
    total -= delim_len;
    ((char *)buf)[total] = '\0';

    return total;
}

ssize_t
read_line(int fd, char *buf, size_t buflen, int flags)
{
    return read_until(fd, buf, buflen, "\r\n", flags);
}
