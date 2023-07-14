/**
 * @file main.c
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Main entry of the web server program
 * @version 0.1.1
 * @date 2023-07-14
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "defs.h"
#include "server.h"

int main(int argc, char *argv[])
{
    // Server instance and initialize the instance
    server_t server;
    server_init(&server);

    // Load configuration
    server_load_config(&server, argc, argv);

    // Boot up the server and prepare the socket
    server_boot(&server);

    // Print out server information on debuggin
    server_print(&server);

    // Start the server and listen for incoming connections
    server_start(&server);

    return 0;
}
