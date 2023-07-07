#include "util.h"

#define BUFSIZE 1024

extern struct configopt conf;

int _portguard(int p)
{
    return (p < MINPORT) && (p > MAXPORT) ? DEFAULTPORT : p;
}

int _nothreadguard(int n)
{
    return (n < MINNOTHREADS) && (n > MAXNOTHREADS) ? DEFAULTNOTHREADS : n;
}

void _lineto(struct configopt *conf, const char *buf, int *status)
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

void lower(char *str)
{
    if (str == NULL)
        return;

    for (size_t i = 0; str[i] != '\0'; ++i)
    {
        str[i] = tolower(str[i]);
    }
}

void upper(char *str)
{
    if (str == NULL)
        return;

    for (size_t i = 0; str[i] != '\0'; ++i)
    {
        str[i] = toupper(str[i]);
    }
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
            _lineto(conf, buffer, &status);

            i = 0;
            memset(buffer, '\0', sizeof(buffer));
        }
        else
            buffer[i++] = c;
    }

    return status;
}

keypair_t getkeypair(const char *raw, const size_t maxlen, const char *delim)
{
    keypair_t kp;
    char *str, *token;
    memset(&kp, 0, sizeof(keypair_t));

    str = strndup(raw, maxlen);
    token = strtok(str, delim);
    strcpy(kp.key, token);

    token = strtok(NULL, delim);
    strcpy(kp.value, token);

    free(str);
    return kp;
}

int buildfilepath(const char *filename, char *result, size_t *len)
{
    snprintf(result, *len, "%s/%s", conf.root, filename);

    return SUCCESS;
}

int checkfile(const char *filename)
{
    struct stat st;

    if (stat(filename, &st) == -1)
        return FAILURE;

    if (!S_ISREG(st.st_mode) || !(S_IRUSR & st.st_mode))
        return FAILURE;

    return SUCCESS;
}
