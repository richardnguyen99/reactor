/**
 * @file main.c
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Main entry point for the Reactor server
 * @version 0.1
 * @date 2023-07-17
 *
 * @copyright Copyright (c) 2023
 */

#include "reactor.h"

int
main(int argc, char *argv[])
{
    int status = SUCCESS;

    // Load server configuration from command line arguments
    struct reactor *server = reactor_init(argc, argv);
    if (server == NULL)
        goto safe_exit;

    // Load configuration into the server instance
    if ((status = reactor_load(server)) != SUCCESS)
        goto safe_exit;

    printf("Listening on %s:%d\n", server->ip, server->port.number);

    // Boot the server instance
    if ((status = reactor_boot(server)) != SUCCESS)
        goto safe_exit;

    // Main loop
    reactor_run(server);

safe_exit:
    if (status == FAILURE)
        fprintf(stderr, "Failure from %s\n", strerror(errno));

    if (status == ERROR)
        fprintf(stderr, "Error: %s\n", strerror(errno));

    reactor_destroy(server);

    return status;
}
