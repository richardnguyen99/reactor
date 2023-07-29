// Private methods for reactor.c

#include "reactor.h"
#include "route.h"

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

int _require_host_header(const char *value)
{
    // TODO: Maybe check localhost
    if (strncmp(value, "localhost", 9) == 0)
        return SUCCESS;
    
    if (strncmp(value, "0.0.0.0", 7) == 0)
        return SUCCESS;

    if (strncmp(value, "127.0.0.1", 9) == 0)
        return SUCCESS;
    
    return FAILURE;
}

void *
_handle_request(void *arg)
{
    struct thread_pool *pool = (struct thread_pool *)arg;
    int http_status, ffd;
    char *buf, *content_type;
    struct stat st;
    size_t len;


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

        if (rev == NULL)
            continue;
        
        if (rev->req == NULL)
            continue;
        
        if (rev->req->path == NULL)
            continue;

        struct __route r = route_get_handler(rev->req->path);


        if (rev->res == NULL)
            rev->res = response_new();

        // Response cannot be created. Instead of shutting down, just letting
        // the client know that the request is dropped.
        if (rev->res == NULL)       
            continue;

        if (rev->req->headers != NULL)
            rev->res->accepts = http_require_accept(rev->req->headers);

        printf("Method: %s\n", GET_HTTP_METHOD(rev->req->method));

        if ((rev->req->method & HTTP_METHOD_GET) == 1 && r.handler.get != NULL)
            r.handler.get(rev->req, rev->res);
        else if ((rev->req->method & HTTP_METHOD_POST) == 1 && r.handler.post != NULL)
            r.handler.post(rev->req, rev->res);
        else if ((rev->req->method & HTTP_METHOD_HEAD) == 1 && r.handler.head != NULL)
            r.handler.head(rev->req, rev->res);
        else if ((rev->req->method & HTTP_METHOD_PUT) == 1 && r.handler.put != NULL)
            r.handler.put(rev->req, rev->res);
        else if ((rev->req->method & HTTP_METHOD_DELETE) == 1 && r.handler.delete != NULL)
            r.handler.delete(rev->req, rev->res);
        else
            response_send_method_not_allowed(rev->res, rev->req->method, rev->req->path);


    send_response:
        if (revent_mod(rev, EPOLLOUT) == ERROR)
            DIE("(handle_request) revent_mod");
    }

    return NULL;
}
