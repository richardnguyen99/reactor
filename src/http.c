#include "http.h"
#include "util.h"

int http_req(int fd)
{
    debug("Handle request: %d\n", fd);
    char buf[BUFSIZ];
    ssize_t n;
    size_t total = 0;

    for (;;)
    {
        n = read_line(fd, buf, BUFSIZ);

        if (errno == EAGAIN)
            continue;

        if (n == -1)
            return -1;
       
        write(STDOUT_FILENO, buf, n);

        total += (ssize_t)n;

        if (n == 2 && strncmp(buf, "\r\n", 2) == 0)
            break;
    }

    debug("Close request: %d\n", fd);
    close(fd);

    return total;
}
