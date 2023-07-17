#include "reactor.h"

struct reactor* reactor_init(int argc, char *argv[])
{
    struct reactor *server = malloc(sizeof(struct reactor));

    if (server == NULL)
        DIE("(reactor_init) malloc");

    server->port = -1;
    server->server_fd = -1;

    memset(server->ip, '\0', INET_ADDRSTRLEN);

    debug("Initialize reactor instance\n");

    return server;
}

void reactor_load(struct reactor *server)
{
    return;
}

void reactor_destroy(struct reactor *server)
{
    if (server->server_fd != -1)
        close(server->server_fd);

    free(server);
}
