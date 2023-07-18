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
    int status             = SUCCESS;
    struct reactor *server = reactor_init(argc, argv);

    if ((status = reactor_load(server)) != SUCCESS)
        goto safe_exit;

    printf("Listening on %s:%d\n", server->ip, server->port.number);

safe_exit:
    reactor_destroy(server);

    return status;
}
