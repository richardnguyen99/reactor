#include "util.h"
#include "http.h"

const char *const endpoints[] = {
    "/",
    "index.html",
    "login",
    "register",
    "about",
};

int parsepath(const char *path, char *endpoint, char *query)
{
    if (strcmp(path, "/") == 0)
    {
        strcpy(endpoint, "/");
        strcpy(query, "");
        return SUCCESS;
    }

    int status = SUCCESS;
    char *token = NULL;
    char *saveptr = NULL;
    char *pathcpy = strdup(path);

    if (pathcpy == NULL)
        return ERROR;

    strcpy(pathcpy, path);

    token = strtok_r(pathcpy, "/", &saveptr);

    if (token == NULL)
    {
        status = ERROR;
        goto cleanup;
    }

    strcpy(endpoint, token);

    token = strtok_r(NULL, "/", &saveptr);

    if (token != NULL)
        strcpy(query, token);

cleanup:
    free(pathcpy);
    return status;
}

int parsestartline(const char *line, req_t *req)
{
    char *token = NULL;
    char *saveptr = NULL;
    char *method = NULL;
    char *path = NULL;
    char *version = NULL;

    token = strtok_r((char *)line, " ", &saveptr);
    method = token;

    if (method == NULL)
        return ERROR;

    token = strtok_r(NULL, " ", &saveptr);
    path = token;

    if (path == NULL)
        return ERROR;

    token = strtok_r(NULL, " ", &saveptr);
    version = token;

    if (version == NULL)
        return ERROR;

    if (strcmp(method, "get") == 0)
        req->method = HTTP_METHOD_GET;
    else if (strcmp(method, "head") == 0)
        req->method = HTTP_METHOD_HEAD;
    else if (strcmp(method, "post") == 0)
        req->method = HTTP_METHOD_POST;
    else if (strcmp(method, "put") == 0)
        req->method = HTTP_METHOD_PUT;
    else if (strcmp(method, "delete") == 0)
        req->method = HTTP_METHOD_DELETE;
    else
        req->method = HTTP_METHOD_UNSUPPORTED;

    req->path = (char *)malloc(sizeof(path));

    if (req->path == NULL)
        return ERROR;

    req->query = (char *)malloc(sizeof(path));
    if (req->query == NULL)
        return ERROR;

    if (parsepath(path, req->path, req->query) == ERROR)
        return ERROR;

    strncpy(req->version, version, strlen(version) - 2);

    return SUCCESS;
}

int parseheaders(const char *raw, req_t *req)
{
    char *token = NULL;
    char *saveptr = NULL;
    char *key = NULL;
    char *value = NULL;
    int status = SUCCESS;
    const char *delim = ": ";

    // Remove the trailing \r\n from the request line
    char *line = strndup(raw, strlen(raw) - 2);
    if (line == NULL)
        return ERROR;

    token = strtok_r((char *)line, delim, &saveptr);
    key = token;

    token = strtok_r(NULL, delim, &saveptr);
    value = token;

    if (key == NULL || value == NULL)
    {
        status = ERROR;
        goto cleanup;
    }

    if (sizeof(value) >= HTTP_LIMIT_REQUEST_LINE)
    {
        status = FAILURE;
        goto cleanup;
    }

    hashmap_put(req->headers, key, value);

cleanup:
    if (line != NULL)
        free(line);
    return status;
}

int checkpath(const char *path)
{
    size_t i = 0;
    int status = FAILURE;

    for (; i < sizeof(endpoints) / sizeof(endpoints[0]); i++)
    {
        if (strcmp(path, endpoints[i]) == 0)
        {
            status = SUCCESS;
            break;
        }
    }

    return status;
}

void checkstartline(req_t *req)
{
    if (req->method == HTTP_METHOD_UNSUPPORTED)
        req->status = HTTP_NOTALLOWED;
    else if (checkpath(req->path) == ERROR)
        req->status = HTTP_BADREQUEST;
    else if (checkpath(req->path) == FAILURE)
        req->status = HTTP_BADREQUEST;
    else if (strcmp(req->version, HTTP_VERSION) != 0)
        req->status = HTTP_UNSUPPORTED;
    else
        req->status = HTTP_SUCCESS;
}

int endofhdr(const char *msgbuf, const size_t len)
{
    return len >= 2 && msgbuf[len - 2] == '\r' && msgbuf[len - 1] == '\n';
}

int endofmsg(const char *msgbuf, const size_t len)
{
    return len == 2 && msgbuf[0] == '\r' && msgbuf[1] == '\n';
}

/* Read one line sent by `fd` socket and store into `msgbuf`.

On success, it returns the number of read bytes if there is any content. On
error, it will returns -1.
*/
ssize_t readline(int fd, char *msgbuf)
{
    size_t n = 0;
    ssize_t nread = 0;

    for (;;)
    {
        nread = recv(fd, (void *)(msgbuf + n), 1, 0);

        if (nread == -1)
            return -1;

        n += (size_t)nread;

        if (endofhdr(msgbuf, n))
            break;
    }

    msgbuf[n] = '\0';

    return n;
}

ssize_t reqread(int fd, req_t *req)
{
    ssize_t nread = 0;
    size_t line = 0, n = 0;
    char buffer[MSGSIZE];
    int status = 0;
    memset(buffer, '\0', MSGSIZE);

    for (; req->status == HTTP_SUCCESS;)
    {
        nread = readline(fd, buffer);

        if (nread == -1)
            return -1;

        lower(buffer);

        if (line == 0)
        {
            if (parsestartline(buffer, req) == ERROR)
                req->status = HTTP_BADREQUEST;
            else
                checkstartline(req);
        }

        if (line != 0 && nread > 2 && req->status != HTTP_ENTITIYTOOLARGE)
        {
            status = parseheaders(buffer, req);
            if (status == ERROR)
                req->status = HTTP_BADREQUEST;
            else if (status == FAILURE)
                req->status = HTTP_ENTITIYTOOLARGE;
        }

        n += (size_t)nread;
        line++;

        if (endofmsg(buffer, (size_t)nread))
            break;
    }

    dprintf(stdout, "End of message\n");
    dprintf(stdout, "Byte read: %ld\n", n);
    dprintf(stdout, "==================\n\n");
    return n;
}

req_t *reqinit(void)
{
    req_t *req = (req_t *)malloc(sizeof(req_t));
    if (req == NULL)
        return NULL;

    req->path = NULL;
    req->body = NULL;
    req->hostname = NULL;
    req->query = NULL;
    req->status = HTTP_SUCCESS;

    req->headers = hashmap_new(strhash, strcmp);
    if (req->headers == NULL)
    {
        free(req);
        return NULL;
    }

    return req;
}

void reqfree(req_t *req)
{
    if (req == NULL)
        return;

    if (req->path != NULL)
        free(req->path);

    if (req->hostname != NULL)
        free(req->path);

    if (req->body != NULL)
        free(req->body);

    if (req->query != NULL)
        free(req->query);

    if (req->headers != NULL)
        hashmap_delete(req->headers);

    free(req);
}

int handle(int fd, char *ipaddr)
{
    ssize_t nread;
    int status = 0;

    req_t *req = reqinit();

    if (req == NULL)
    {
        status = -1;
        goto safe_exit;
    }

    memcpy(req->ip, ipaddr, strlen(ipaddr));
    nread = reqread(fd, req);

#ifdef DEBUG
    hashmap_iterate(hashmap_begin(req->headers), NULL, "http.headers");
    fprintf(stdout, "%d %s %s\n", req->method, req->path, req->version);
    fprintf(stdout, "IP: %s\n\n", req->ip);
    fprintf(stdout, "Status code: %d\n", req->status);

#endif

    if (nread == -1)
    {
        perror("recv");
        status = (int)nread;
        goto safe_exit;
    }

safe_exit:
    if (req != NULL)
        reqfree(req);
    close(fd);
    return status;
}
