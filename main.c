/**
 * @file main.c
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Main entry point for the Reactor server
 * @version 0.2
 * @date 2023-07-17
 *
 * @copyright Copyright (c) 2023
 */

#include <rx_defs.h>
#include <rx_core.h>

#include "reactor.h"
#include "route.h"

void
ping_get_handler(struct request *req, struct response *res)
{
    int accept_ret;

    if ((accept_ret = response_accept(res, "json")) == ERROR)
        return;

    if (accept_ret == HTTP_CONTENT_TYPE_INVALID)
        return;

    res->status       = HTTP_SUCCESS;
    res->method       = HTTP_METHOD_GET;
    res->content_type = accept_ret;

    response_json(res, "{\r\n"
                       "\"message\": \"Hey, I\'m alive!\",\r\n"
                       "\"status\": 200\r\n"
                       "}\r\n");
}

void
index_get_handler(struct request *req, struct response *res)
{
    int accept_ret;
    // There must be some errors within the server
    if ((accept_ret = response_accept(res, "hmtl")) == ERROR)
        return;

    if (accept_ret == HTTP_CONTENT_TYPE_INVALID)
        return;

    res->status       = HTTP_SUCCESS;
    res->method       = HTTP_METHOD_GET;
    res->content_type = accept_ret;

    response_send_file(res, "index.html");
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
main(int argc, char *const *argv)
{
    struct rx_daemon daemon;
    memset(&daemon, 0, sizeof(struct rx_daemon));

    (void)rx_daemon_init(&daemon, argc, argv);

    return 0;
}
