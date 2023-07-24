// Private methods for reactor.c

#include "reactor.h"

void
__construct_response(struct response *res, int status, char *body,
                     size_t body_len)
{
    res->status      = status;
    res->status_text = http_get_status_text(status);
    res->body        = body;
    res->body_len    = body_len;
    res->version     = strdup("HTTP/1.1");
}

int
__validate_response(struct response *res, char *body)
{
    if (res == NULL)
        return ERROR;

    if (res->status_text == NULL)
        return ERROR;

    if (body != NULL && res->body == NULL)
        return ERROR;

    if (body != NULL && res->body_len == 0)
        return ERROR;

    if (res->version == NULL)
        return ERROR;

    return SUCCESS;
}

int
_prepare_socket(char *host, const char *service)
{
    struct addrinfo hints, *results, *rp;
    int status, fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(NULL, service, &hints, &results);

    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    for (rp = results; rp != NULL; rp = rp->ai_next)
    {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (fd == -1)
            continue;

        // clang-format off

        status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 
                            &(int){1}, sizeof(int));

        // clang-format on       

        if (status == -1)
        {
            close(fd);
            continue;
        }

        if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(fd);
    }

    if (rp == NULL)
        DIE("(prepare_socket) bind");

    inet_ntop(rp->ai_family, &((struct sockaddr_in *)rp->ai_addr)->sin_addr,
              host, INET_ADDRSTRLEN);

safe_exit:
    freeaddrinfo(results);
    return fd;
}

int
_set_nonblocking(int fd)
{
    int flags, status;

    // Get the current flags
    flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
        return ERROR;

    // Append the non-blocking flag to the current flags
    flags |= O_NONBLOCK;
    status = fcntl(fd, F_SETFL, flags);

    if (status == -1)
        return ERROR;

    return SUCCESS;
}

void *
_handle_request(void *arg)
{
    struct thread_pool *pool = (struct thread_pool *)arg;
    int status;
    char *buf = NULL;
    struct stat st;
    int ffd;

    for (;;)
    {
        if (sem_wait(&(pool->full)) == -1)
            DIE("(handle_request) sem_wait");
        if (pthread_mutex_lock(&(pool->lock)) == -1)
            DIE("(handle_request) pthread_mutex_lock");

        struct reactor_event *rev = rbuffer_pop(pool->buffer);

        if (pthread_mutex_unlock(&(pool->lock)) == -1)
            DIE("(handle_request) pthread_mutex_unlock");
        if (sem_post(&(pool->empty)) == -1)
            DIE("(handle_request) sem_post");

        struct route route = http_get_uri_handle(rev->req->path);
        rev->res = response_new();

         if (route.uri == NULL)
         {
             __construct_response(rev->res, HTTP_NOT_FOUND, NULL, 0);
             __validate_response(rev->res, NULL);

             continue;
         }

         if (rev->req->method & route.methods == 0)
         {
                __construct_response(rev->res, HTTP_METHOD_NOT_ALLOWED, NULL, 0);
                __validate_response(rev->res, NULL);
    
                continue;
         }

        if ((ffd = open(route.resource, O_RDONLY, 0)) == -1)
            DIE("(handle_request) open");

        if (fstat(ffd, &st) == -1)
            DIE("(handle_request) fstat");

        buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, ffd, 0);

        if (buf == MAP_FAILED)
            DIE("(handle_request) mmap");

        rev->res->status      = HTTP_SUCCESS;
        rev->res->status_text = http_get_status_text(HTTP_SUCCESS);
        rev->res->version     = NULL;
        rev->res->body        = buf;
        rev->res->body_len    = st.st_size;

        if (revent_mod(rev, EPOLLOUT) == ERROR)
            DIE("(handle_request) revent_mod");
    }

    return NULL;
}
