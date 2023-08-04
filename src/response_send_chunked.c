#include "response.h"

int
__send_chunk_headers(struct response *res, int fd)
{
    ssize_t nsent = 0;

    if (res->__chunked_buf[0] == '\0')
    {
        res->__chunked_size =
            snprintf(res->__chunked_buf, CHUNKSIZE,
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: %s\r\n"
                     "Connection: keep-alive\r\n"
                     "Transfer-Encoding: chunked\r\n"
                     "Keep-Alive: timeout=5, max=1000\r\n"
                     "Server: reactor/%s\r\n"
                     "\r\n",
                     res->status, GET_HTTP_MSG(res->status),
                     GET_HTTP_CONTENT_TYPE(res->content_type), REACTOR_VERSION);
    }

    for (; res->__chunked_offset < res->__chunked_size;)
    {
        nsent = send(fd, res->__chunked_buf + res->__chunked_offset,
                     res->__chunked_size - res->__chunked_offset, MSG_DONTWAIT);

        if (nsent == -1 && errno == EAGAIN)
            return EAGAIN;

        if (nsent == -1 && errno == EPIPE)
            return EPIPE;

        if (nsent == -1)
            DIE("(response_send_chunked) send headers");

        res->__chunked_offset += (size_t)nsent;
    }

    res->__chunked_offset = 0;    // Reset the offset
    res->__chunked_buf[0] = '\0'; // Clear the buffer
    res->__chunked_state  = 1;    // Ready to send chunks
    printf("Go to send chunks\n");

    return SUCCESS;
}

int
__send_chunks(struct response *res, int fd)
{
    ssize_t nsent = 0;

    for (; res->__chunked_offset < res->body_len;)
    {
        if (res->__chunked_buf[0] == '\0')
        {
            size_t __n;
            res->__chunked_size =
                res->body_len - res->__chunked_offset > CHUNKSIZE
                    ? CHUNKSIZE                              // Max chunk
                    : res->body_len - res->__chunked_offset; // Left chunk

            snprintf(res->__chunked_buf, CHUNKHDR, "%lx\r\n",
                     res->__chunked_size);

            __n = strlen(res->__chunked_buf);
            printf("\nChunk size (%ld): %s", __n, res->__chunked_buf);

            snprintf(res->__chunked_buf + __n, res->__chunked_size + 1, "%s",
                     res->body + res->__chunked_offset);

            __n += res->__chunked_size;
            res->__chunked_offset += res->__chunked_size;

            snprintf(res->__chunked_buf + __n, CHUNKSIZE, "\r\n");
            __n += 2;

            printf("Chunk offset: %ld\n", res->__chunked_offset);

            res->__chunked_size = __n;
            res->__chunked_sent = 0;
            printf("End chunk\n");
        }

        printf("Sending chunk\n");
        for (; res->__chunked_sent < res->__chunked_size;)
        {
            nsent = send(fd, res->__chunked_buf + res->__chunked_sent,
                         res->__chunked_size - res->__chunked_sent,
                         MSG_DONTWAIT | MSG_NOSIGNAL);

            if (nsent == -1 && errno == EAGAIN)
                return EAGAIN;

            if (nsent == -1 && errno == EPIPE)
                return EPIPE;

            if (nsent < 0 && errno == ECONNRESET)
                return SUCCESS;

            if (nsent == -1)
                DIE("(response_send_chunked) send chunk");

            res->__chunked_sent += (size_t)nsent;

            if (res->__chunked_sent == res->__chunked_size)
                break;
        }
        printf("Sent chunk\n");

        res->__chunked_buf[0] = '\0'; // clear buffer & send a new chunk
        res->__chunked_sent   = 0;
    }

    res->__chunked_state = 2;
    res->__chunked_sent  = 0;

    return SUCCESS;
}

int
__send_terminal_chunk(struct response *res, int fd)
{
    ssize_t nsent = 0;

    for (; res->__chunked_sent < 5;)
    {
        nsent = send(fd, "0\r\n\r\n", 5, MSG_DONTWAIT | MSG_NOSIGNAL);

        if (nsent == -1 && errno == EAGAIN)
            return EAGAIN;

        if (nsent == -1 && errno == EPIPE)
            return EPIPE;

        if (nsent == -1)
            DIE("(response_send_chunked) send end of chunk");

        res->__chunked_sent += (size_t)nsent;
    }
    printf("Sent terminal chunk\n");

    res->__chunked_state = 3;

    return SUCCESS;
}

int
response_send_chunked(struct response *res, int fd)
{
    int status = SUCCESS;

    if (res->__chunked_state == 0)
    {
        printf("Sending chunk headers\n");
        status = __send_chunk_headers(res, fd);

        if (status == EAGAIN || status == EPIPE)
            return status;
    }

    if (res->__chunked_state == 1)
    {
        printf("Sending chunks\n");
        status = __send_chunks(res, fd);

        if (status == EAGAIN || status == EPIPE)
            return status;
    }

    if (res->__chunked_state == 2)
    {
        printf("Sending terminal chunk\n");
        status = __send_terminal_chunk(res, fd);

        if (status == EAGAIN || status == EPIPE)
            return status;
    }

    printf("Finished sending chunks\n");
    return SUCCESS;
}
