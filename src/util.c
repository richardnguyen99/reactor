#include "util.h"

ssize_t read_line(int fd, char *buf, size_t n)
{
    ssize_t nread = 0;
    size_t total = 0;

    for (;total < n;)
    {
        // Read one byte at a time to check for newline
        nread = read(fd, buf + total, 1);

        if (nread == -1)
            return -1;

        total += (size_t)nread;

        if (nread == 0 || *(buf + total - 1) == '\n')
            break;
    }

    // Remove newline character
    *(buf + total - 1) = '\0';

    return (ssize_t)total;
}
