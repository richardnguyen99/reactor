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

    //if (rsk->res == NULL)
        //rsk->res = response_new();

    //res = rsk->res;

    //response_status(res, HTTP_REQUEST_TIMEOUT);
    //response_text(res, "408: Request timeout\n", 24);

    // ret = revent_mod(rev_sock, EPOLLOUT);

    // if (ret == ERROR)
    // {
        // revent_destroy(rev_sock);
        // revent_destroy(rtm);
    // }
}

void *
_handle_request(void *arg)
{
    struct thread_pool *pool = (struct thread_pool *)arg;
    struct thread_task *task;
    struct reactor_sock *rsk;
    struct request *req;
    struct response *res;
    struct http_obj *http;

    int status;

    for (;;)
    {
        if (sem_wait(&(pool->full)) == -1)
            DIE("(handle_request) sem_wait");
        if (pthread_mutex_lock(&(pool->lock)) == -1)
            DIE("(handle_request) pthread_mutex_lock");

        task = rbuffer_pop(pool->buffer);

        if (pthread_mutex_unlock(&(pool->lock)) == -1)
            DIE("(handle_request) pthread_mutex_unlock");
        if (sem_post(&(pool->empty)) == -1)
            DIE("(handle_request) sem_post");
        
        http = http_new();
        req = request_new();
        res = response_new();

        if (http == NULL || req == NULL || res == NULL)
        {
            status = HTTP_INTERNAL_SERVER_ERROR;
            goto send_response;
        }

        http->req = req;
        http->res = res;
        http->cfd = task->rev->data.rsk->fd;

        // Parse the request line
        status = http_request_line(http);
        if (status == 0)
        {
            task->rev->refcnt--;
            revent_destroy(task->rev);
            goto end_request;
        }

        if (status != HTTP_SUCCESS)
            goto send_response;

        // Parse the request headers
        status = http_request_headers(http);
        if (status != HTTP_SUCCESS)
            goto send_response;

        //pthread_rwlock_wrlock(&(res->rwlock));
        if (res->accepts == NULL)
            res->accepts = http_require_accept(req->headers);
        //pthread_rwlock_unlock(&(res->rwlock));

        struct __route route = route_get_handler(req->path);

        // If the route is not found, send 404
        status = route.status;
        if (status == HTTP_NOT_FOUND)
            goto send_response;

        if ((req->method & HTTP_METHOD_GET) == 1 && route.handler.get != NULL)
            route.handler.get(req, res);

        else if ((req->method & HTTP_METHOD_POST) == 1 && route.handler.post != NULL)
            route.handler.post(req, res);

        else if ((req->method & HTTP_METHOD_HEAD) == 1 && route.handler.head != NULL)
            route.handler.head(req, res);

        else if ((req->method & HTTP_METHOD_PUT) == 1 && route.handler.put != NULL)
            route.handler.put(req, res);

        else if ((req->method & HTTP_METHOD_DELETE) == 1 && route.handler.delete != NULL)
            route.handler.delete(req, res);

        else
            status = HTTP_METHOD_NOT_ALLOWED;

    send_response:
        http_response_status(http, status);

        debug("Status: %d\n", http->res->status);
        debug("Method: %s\n", GET_HTTP_METHOD(req->method));
        debug("Path: %s\n", req->path);

        http_response_send(http);

        debug("Response sent: %ld\n", http->res->body_len);
        debug("\n");


    free_http:
        task->rev->refcnt--;

        http_free(http);

        req = NULL;
        res = NULL;
        http = NULL;

        free(task);

    end_request:
        revent_mod(task->rev, EPOLLOUT);

        debug("\
=================================EPOLLOUT=======================================\n\
");
    }

    return NULL;
}
