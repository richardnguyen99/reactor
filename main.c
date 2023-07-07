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

#define _GNU_SOURCE

#include <dirent.h>

#include "util.h"
#include "socket.h"
#include "http.h"

struct configopt conf;

char **getfiles()
{
    char **files = NULL;
    int i = 0;
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(conf.root)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type == DT_REG)
            {
                files = realloc(files, sizeof(char *) * (i + 1));
                files[i] = malloc(sizeof(char) * (strlen(ent->d_name) + 1));
                strcpy(files[i], ent->d_name);
                i++;
            }
        }
        closedir(dir);
    }
    else
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    files[i] = NULL;

    return files;
}

int main(void)
{
    char hostbuf[NI_MAXHOST], servbuf[NI_MAXSERV];
    int status, lfd, cfd;

    memset(&conf, '\0', sizeof(struct configopt));
    if (readenv(".env", &conf) == JENV_RDERROR)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    conf.resources = getfiles();

    dprintf(stdout, "Root dir: %s\n", conf.root);
    dprintf(stdout, "Port number: %ld\n", conf.port);
    dprintf(stdout, "Number threads: %ld\n", conf.num_thread);

    lfd = bindsocket(conf.port);

    for (;;)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(struct sockaddr_in));

        cfd = accept(lfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (cfd == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        memset(hostbuf, '\0', sizeof(hostbuf));
        memset(servbuf, '\0', sizeof(servbuf));

        status = getnameinfo(
            (const struct sockaddr *)&client_addr,
            client_addr_len,
            hostbuf, sizeof(hostbuf),
            servbuf, sizeof(servbuf),
            (NI_NUMERICHOST | NI_NUMERICSERV));

        if (status != 0)
        {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
            exit(EXIT_FAILURE);
        }

        dprintf(stdout, "Connected: %s:%s\n", hostbuf, servbuf);
        handle(cfd, hostbuf);
    }

    for (size_t i = 0; conf.resources[i] != NULL; i++)
    {
        free(conf.resources[i]);
    }
    free(conf.resources);

    return EXIT_SUCCESS;
}
