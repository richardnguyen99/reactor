#include "util.h"

int read_config(const char *filename, config_t *config)
{
    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char buf[BUFSIZE];
    ssize_t nread = read(fd, buf, BUFSIZE);

    if (nread == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    char *line = strtok(buf, "\n");

    while (line != NULL)
    {
        char *key = strtok(line, " ");
        char *value = strtok(NULL, " ");

        if (strcmp(key, "root") == 0)
        {
            strcpy(config->root, value);
        }
        else if (strcmp(key, "port") == 0)
        {
            strcpy(config->port, value);
        }
        else if (strcmp(key, "nthreads") == 0)
        {
            config->nthreads = atoi(value);
        }

        line = strtok(NULL, "\n");
    }

    return 0;
}
