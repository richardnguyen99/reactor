#include "util.h"
#include "http.h"

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

ssize_t readreq(int fd, http_headers_t headers)
{
    ssize_t nread = 0;
    size_t line = 0, n = 0;
    char buffer[MSGSIZE];
    memset(buffer, '\0', MSGSIZE);

    for (;;)
    {
        nread = readline(fd, buffer);

        if (nread == -1)
            return -1;

        lower(buffer);
        // sprintf(buffer, "%s\n", buffer);
        // write(STDOUT_FILENO, (const void *)buffer, (size_t)nread);

        if (line != 0 && nread > 2)
        {
            keypair_t kv = getkeypair(buffer, nread - 2, ": ");
            hashmap_put(headers, kv.key, kv.value);
        }

        n += (size_t)nread;
        line++;

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

    http_headers_t headers = hashmap_new(strhash, strcmp);

    nread = readreq(fd, headers);

#ifdef DEBUG
    hashmap_iterate(hashmap_begin(headers), NULL, "http.headers");
#endif

    if (nread == -1)
    {
        perror("recv");
        status = (int)nread;
        goto safe_exit;
    }

safe_exit:
    close(fd);
    hashmap_delete(headers);
    return status;
}
