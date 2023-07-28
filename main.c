/**
 * @file main.c
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Main entry point for the Reactor server
 * @version 0.2
 * @date 2023-07-17
 *
 * @copyright Copyright (c) 2023
 */

#include "reactor.h"
#include "route.h"

void
ping_get_handler(struct request *req, struct response *res)
{
    return;
}

void
index_get_handler(struct request *req, struct response *res)
{
    return;
}

void
index_head_handler(struct request *req, struct response *res)
{
    return;
}

void
about_get_handler(struct request *req, struct response *res)
{
    return;
}

const struct __route router_table[] = {
    {"/",
     "index.html",   {.get    = index_get_handler,
      .head   = NULL,
      .post   = NULL,
      .delete = NULL,
      .put    = NULL},
     -1                                    },

    {"/ping",
     NULL,           {
         .get    = ping_get_handler,
         .post   = NULL,
         .delete = NULL,
         .put    = NULL,
         .head   = NULL,
     },                        -1},

    {"/about",
     NULL,           {.get    = about_get_handler,
      .head   = NULL,
      .post   = NULL,
      .delete = NULL,
      .put    = NULL},
     -1                                    },

    {NULL,     NULL, {0},                -1}
};

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
