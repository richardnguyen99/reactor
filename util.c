#include "util.h"

#define BUFSIZE 1024

int _portguard(int p)
{
    return (p < MINPORT) && (p > MAXPORT) ? DEFAULTPORT : p;
}

int _nothreadguard(int n)
{
    return (n < MINNOTHREADS) && (n > MAXNOTHREADS) ? DEFAULTNOTHREADS : n;
}

void _lineto(struct configopt *conf, const char *buf, size_t len, int *status)
{
    char *token;

    token = strchr(buf, '=');

    if (token == NULL)
    {
        *status = -2;
        return;
    }

    if (strncmp(buf, "ROOT", 4) == 0)
        strcpy(conf->root, token + 1);

    if (strncmp(buf, "PORT", 4) == 0)
        conf->port = _portguard(atoi(token + 1));

    if (strncmp(buf, "NO_THREADS", 10) == 0)
        conf->num_thread = _nothreadguard(atoi(token + 1));
}

/* Read env variables from `filename` and store at `conf`. On success, it
 returns 0 if there is no error and all read successfully.

  If there is no file or the file coud not be read, -1 is returned (perror).

  If there is something with env file, -2 will be returned (user).
 */
int readenv(const char *filename, struct configopt *conf)
{
    ssize_t nread;
    char c, buffer[BUFSIZE];
    int status, fd, i;

    fd = open(filename, O_RDONLY);

    if (fd == -1)
        return JENV_RDERROR;

    i = 0;
    status = 0;
    nread = 0;
    while ((nread = read(fd, &c, 1)) > 0)
    {
        if (c == '\n')
        {
            buffer[i] = '\0';
            _lineto(conf, buffer, i, &status);

            i = 0;
            memset(buffer, '\0', sizeof(buffer));
        }
        else
            buffer[i++] = c;
    }

    return status;
}
