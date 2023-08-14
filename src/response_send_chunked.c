#include "response.h"

const int sflag = MSG_DONTWAIT | MSG_NOSIGNAL;

int
__send_chunk_headers(struct response *res, int fd)
{
    ssize_t nsent = 0;

    if (res->__chunked_buf[0] == '\0')
    {
        res->__chunked_size =
            snprintf(res->__chunked_buf, MAX_HDR_LEN,
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: %s\r\n"
                     "Connection: close\r\n"
                     "Transfer-Encoding: chunked\r\n"
                     "Server: reactor/%s\r\n"
                     "\r\n",
                     res->status, GET_HTTP_MSG(res->status),
                     GET_HTTP_CONTENT_TYPE(res->content_type), REACTOR_VERSION);
    }

    for (; res->__chunked_offset < res->__chunked_size;)
    {
        nsent = send(fd, res->__chunked_buf + res->__chunked_offset,
                     res->__chunked_size - res->__chunked_offset, sflag);

        if (nsent == -1 && errno == EAGAIN)
            return EAGAIN;

        if (nsent == -1 && (errno == EPIPE || errno == EBADF))
            return EPIPE;

        if (nsent == -1)
            DIE("(response_send_chunked) send headers");

        res->__chunked_offset += (size_t)nsent;
    }

    res->__chunked_offset = 0;    // Reset the offset
    res->__chunked_buf[0] = '\0'; // Clear the buffer
    res->__chunked_state  = 1;    // Ready to send chunks

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
                res->body_len - res->__chunked_offset > MAX_CHK_LEN
                    ? MAX_CHK_LEN                            // Max chunk
                    : res->body_len - res->__chunked_offset; // Left chunk

            snprintf(res->__chunked_buf, MAX_CHK_LEN, "%lx\r\n",
                     res->__chunked_size);

            __n = strlen(res->__chunked_buf);

            snprintf(res->__chunked_buf + __n, res->__chunked_size + 1, "%s",
                     res->body + res->__chunked_offset);

            __n += res->__chunked_size;
            res->__chunked_offset += res->__chunked_size;

            snprintf(res->__chunked_buf + __n, MAX_CHK_LEN, "\r\n");
            __n += 2;

            res->__chunked_size = __n;
            res->__chunked_sent = 0;
        }

        for (; res->__chunked_sent < res->__chunked_size;)
        {
            nsent = send(fd, res->__chunked_buf + res->__chunked_sent,
                         res->__chunked_size - res->__chunked_sent, sflag);

            if (nsent == -1 && errno == EAGAIN)
                return EAGAIN;

            if (nsent == -1 && (errno == EPIPE || errno == EBADF))
                return EPIPE;

            if (nsent < 0 && errno == ECONNRESET)
                return SUCCESS;

            if (nsent == -1)
                DIE("(response_send_chunked) send chunk");

            res->__chunked_sent += (size_t)nsent;

            if (res->__chunked_sent == res->__chunked_size)
                break;
        }

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
        nsent = send(fd, "0\r\n\r\n", 5, sflag);

        if (nsent == -1 && errno == EAGAIN)
            return EAGAIN;

        if (nsent == -1 && (errno == EPIPE || errno == EBADF))
            return EPIPE;

        if (nsent == -1)
            DIE("(response_send_chunked) send end of chunk");

        res->__chunked_sent += (size_t)nsent;
    }

    res->__chunked_state = 3;

    return SUCCESS;
}

int
response_send_chunked(struct response *res, int fd)
{
    int status = SUCCESS;

    status = __send_chunk_headers(res, fd);

    status = __send_chunks(res, fd);

    status = __send_terminal_chunk(res, fd);

    return SUCCESS;

response_internal_error:
}
