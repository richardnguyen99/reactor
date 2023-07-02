/**
 * @file main.c
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Main entry for the JAWS executable
 * @version 0.1
 * @date 2023-06-30
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "util.h"
#include "socket.h"

int main(void)
{
    struct configopt conf;
    struct addrinfo hints, *results, *rp;
    int status, lfd;

    memset(&conf, '\0', sizeof(struct configopt));
    if (readenv(".env", &conf) == JENV_RDERROR)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    dprintf(stdout, "Root dir: %s\n", conf.root);
    dprintf(stdout, "Port number: %ld\n", conf.port);
    dprintf(stdout, "Number threads: %ld\n", conf.num_thread);

    lfd = bindsocket(conf.port);

    return EXIT_SUCCESS;
}
