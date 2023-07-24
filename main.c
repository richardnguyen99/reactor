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
    struct reactor *server = NULL;
    reactor_init(&server, argc, argv);

    // Load the server configuration into the server instance
    reactor_load(server);

    debug("Listening on %s:%d\n", server->ip, server->port.number);

    // Boot the server instance
    if ((status = reactor_boot(server)) != SUCCESS)
        goto safe_exit;

    // Main loop
    reactor_run(server);

    // The server is supposed to run forever. If the main loop exits, there is
    // a fatal error that needs to be handled. Therefore, these codes are solely
    // for "best practices".

safe_exit:
    reactor_destroy(server);

    return status;
}
