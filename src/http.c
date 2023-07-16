#include "http.h"
#include "util.h"

int http_req(int fd)
{
    char buf[BUFSIZ];
    size_t total = 0;

    for (;;)
    {
        ssize_t n = read_line(fd, buf + total, BUFSIZ - total);

        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        else if (n == 0)
            return 0;
        else
        {
            if (write(fd, buf + total, n) == -1)
                return -1;
        }

        total += n;

        if (n == 2 && buf[total - 2] == '\r' && buf[total - 1] == '\n')
            break;
    }

    return 0;
}
