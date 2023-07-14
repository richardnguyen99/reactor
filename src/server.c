#include "util.h"
#include "server.h"

void server_print(server_t *server)
{
#ifdef DEBUG
    printf("Hello, world!\n");
#endif

    return;
}


void server_init(server_t *server)
{
    debug("Initializing server instance...\n\n");

    // Initialize the server instance
    server->sockfd = -1;
    server->epollfd = -1;
    server->nthreads = DEFAULT_NTHREADS;
    server->port = DEFAULT_PORT;
    strcpy(server->rootdir, DEFAULT_ROOTDIR);
    server->events = NULL;
    server->threads = NULL;

    return;
}

void server_load_config(server_t *server, int argc, char *argv[])
{
    debug("Loading configuration...\n\n");

    return;
}

void server_boot(server_t *server)
{
    debug("Booting up the server...\n\n");

    debug("Server is ready...\n\n");

    return;
}

void server_start(server_t *server)
{
    debug("Starting the server...\n\n");

    return;
}

