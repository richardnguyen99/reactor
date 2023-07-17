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

int main(int argc, char *argv[])
{
    struct reactor *server = reactor_init(argc, argv);

    reactor_destroy(server);

    return 0;
}
