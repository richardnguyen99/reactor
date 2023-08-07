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

void 
_handle_timer(struct reactor_event *rtm)
{
    int ret;
    struct reactor_event *rev_sock = rtm->data.rtm->rev_socket;
    struct reactor_socket *rsk = rev_sock->data.rsk;
    struct response *res;

    if (rsk->res == NULL)
        rsk->res = response_new();

    res = rsk->res;

    response_status(res, HTTP_REQUEST_TIMEOUT);
    response_text(res, "408: Request timeout\n", 24);

    ret = revent_mod(rev_sock, EPOLLOUT);

    if (ret == ERROR)
    {
        revent_destroy(rev_sock);
        revent_destroy(rtm);
    }
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

        struct reactor_socket *rsk = rev->data.rsk;

        if (rsk == NULL)
            continue;
        
        if (rsk->req == NULL)
            continue;

        pthread_rwlock_wrlock(&(rsk->res_lock));
        if (rsk->res == NULL)
            rsk->res = response_new();
        pthread_rwlock_unlock(&(rsk->res_lock));

        // Response cannot be created. Instead of shutting down, just letting
        // the client know that the request is dropped.
        if (rsk->res == NULL)       
            continue;

        if (rsk->req->status == HTTP_NOT_SET || rsk->req->status == HTTP_INTERNAL_SERVER_ERROR)
        {
            response_send_internal_server_error(rsk->res);
            goto send_response;
        }

        if (rsk->req->status == HTTP_BAD_REQUEST)
        {
            printf("%s\n", rsk->req->path);
            response_send_bad_request(rsk->res);
            goto send_response;
        }
        
        pthread_rwlock_wrlock(&(rsk->res->rwlock));
        if (rsk->res->accepts == NULL)
            rsk->res->accepts = http_require_accept(rsk->req->headers);
        pthread_rwlock_unlock(&(rsk->res->rwlock));

        struct __route route = route_get_handler(rsk->req->path);

        // If the route is not found, send 404
        if (route.status == HTTP_NOT_FOUND)
        {
            response_send_not_found(rsk->res, rsk->req->path);
            goto send_response;
        }

        if ((rsk->req->method & HTTP_METHOD_GET) == 1 && route.handler.get != NULL)
            route.handler.get(rsk->req, rsk->res);

        else if ((rsk->req->method & HTTP_METHOD_POST) == 1 && route.handler.post != NULL)
            route.handler.post(rsk->req, rsk->res);

        else if ((rsk->req->method & HTTP_METHOD_HEAD) == 1 && route.handler.head != NULL)
            route.handler.head(rsk->req, rsk->res);

        else if ((rsk->req->method & HTTP_METHOD_PUT) == 1 && route.handler.put != NULL)
            route.handler.put(rsk->req, rsk->res);

        else if ((rsk->req->method & HTTP_METHOD_DELETE) == 1 && route.handler.delete != NULL)
            route.handler.delete(rsk->req, rsk->res);

        else
            response_send_method_not_allowed(rsk->res, rsk->req->method, rsk->req->path);


    send_response:
        int status;

        status = revent_mod(rev, EPOLLOUT);

        if (status == ERROR)
            revent_destroy(rev);
    }

    return NULL;
}
