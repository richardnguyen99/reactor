#include "socket.h"

int bindsocket(int port)
{
    struct addrinfo hints, *results, *rp;
    char portstr[6];
    int fd, s;

    const int reuseaddr = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     /* Accept IPv4 or IPv6*/
    hints.ai_socktype = SOCK_STREAM; /* Use TCP */
    hints.ai_flags = AI_PASSIVE;     /* Accept new connections (server) */
    hints.ai_protocol = IPPROTO_TCP; /* Explicitly use TCP */

    snprintf(portstr, sizeof(portstr), "%d", port);

    if (getaddrinfo(NULL, portstr, &hints, &results) == -1)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (rp = results; rp != NULL; rp = rp->ai_next)
    {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (fd == -1)
            continue;

        s = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                       (const void *)&reuseaddr,
                       (socklen_t)sizeof(reuseaddr));

        if (s == -1)
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        if (bind(fd, rp->ai_addr, rp->ai_addrlen) == -1)
        {
            close(fd);
            perror("bind");
            continue;
        }

        break;
    }

    if (rp == NULL)
    {
        freeaddrinfo(results);
        fprintf(stderr, "failed to bind socket\n");
        exit(EXIT_FAILURE);
    }

    if (listen(fd, LISTEN_QUEUE) == -1)
    {
        close(fd);
        freeaddrinfo(results);
        perror("listen");
        exit(EXIT_FAILURE);
    }

    char ipstr[INET6_ADDRSTRLEN];
    tohostname(rp->ai_addr, rp->ai_family, ipstr);
    dprintf(stdout, "Server is listen on %s with port %s\n", ipstr, portstr);

    freeaddrinfo(results);

    return fd;
}

int tohostname(struct sockaddr *addr, const int family, char *ipstr)
{
    void *ip;
    dprintf(stdout, "Family: %d\n", family);

    if (family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)addr;
        ip = &(ipv4->sin_addr);
    }
    else if (family == AF_INET6)
    {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)addr;
        ip = &(ipv6->sin6_addr);
    }
    else
        return -1;

    inet_ntop(family, ip, ipstr, sizeof(ipstr));

    return 0;
}
