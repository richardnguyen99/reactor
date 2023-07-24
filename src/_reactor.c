// Private methods for reactor.c

#include "reactor.h"

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

        status =
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

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
        debug("Dequeue: %p\n", rev);
        debug("Queue size: %ld\n", pool->buffer->size);

        if (pthread_mutex_unlock(&(pool->lock)) == -1)
            DIE("(handle_request) pthread_mutex_unlock");

        if (sem_post(&(pool->empty)) == -1)
            DIE("(handle_request) sem_post");

        // status = _check_path(rev);

        if (stat("index.html", &st) == -1)
            DIE("(handle_request) stat");

        ffd = open("index.html", O_RDONLY, 0);

        if (ffd == -1)
            DIE("(handle_request) open");

        buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, ffd, 0);

        if (buf == MAP_FAILED)
            DIE("(handle_request) mmap");

        rev->res = (struct response *)malloc(sizeof(struct response));

        if (rev->res == NULL)
            DIE("(handle_request) malloc");

        rev->res->headers     = NULL;
        rev->res->status      = HTTP_SUCCESS;
        rev->res->status_text = NULL;
        rev->res->version     = NULL;
        rev->res->body        = buf;
        rev->res->body_len    = st.st_size;

        if (revent_mod(rev, EPOLLOUT) == ERROR)
            DIE("(handle_request) revent_mod");
    }

    return NULL;
}
