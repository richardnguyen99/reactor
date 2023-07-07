#include "util.h"
#include "http.h"

const char *const endpoints[] = {
    "/",
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
        return FATAL;

    strcpy(pathcpy, path);

    token = strtok_r(pathcpy, "/", &saveptr);

    if (token == NULL)
    {
        status = FAILURE;
        goto cleanup;
    }

    strcpy(endpoint, token);

    token = strtok_r(NULL, "/", &saveptr);

    if (token != NULL)
        strcpy(query, token);
    else
        strcpy(query, "");

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
    int status = SUCCESS;

    token = strtok_r((char *)line, " ", &saveptr);
    method = token;

    if (method == NULL)
        return FAILURE;

    token = strtok_r(NULL, " ", &saveptr);
    path = token;

    if (path == NULL)
        return FAILURE;

    token = strtok_r(NULL, " ", &saveptr);
    version = token;

    if (version == NULL)
        return FAILURE;

    if (strcmp(method, "GET") == 0)
        req->method = HTTP_METHOD_GET;
    else if (strcmp(method, "HEAD") == 0)
        req->method = HTTP_METHOD_HEAD;
    else if (strcmp(method, "POST") == 0)
        req->method = HTTP_METHOD_POST;
    else if (strcmp(method, "PUT") == 0)
        req->method = HTTP_METHOD_PUT;
    else if (strcmp(method, "DELETE") == 0)
        req->method = HTTP_METHOD_DELETE;
    else
        req->method = HTTP_METHOD_UNSUPPORTED;

    req->path = (char *)malloc(sizeof(path));
    req->query = (char *)malloc(sizeof(path));

    if (req->path == NULL || req->query == NULL)
    {
        status = FATAL;
        goto safe_exit;
    }

    status = parsepath(path, req->path, req->query);
    if (status != SUCCESS)
        goto safe_exit;

    strncpy(req->version, version, strlen(version) - 2);

safe_exit:
    return status;
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

    char *resourcepath = (char *)malloc(sizeof(char) * RSRCSIZE);
    if (resourcepath == NULL)
    {
        status = ERROR;
        goto cleanup;
    }

    char *duppath = (char *)malloc(sizeof(char) * RSRCSIZE);
    if (duppath == NULL)
    {
        status = ERROR;
        goto cleanup;
    }

    if (strcasecmp(path, "/") == 0)
        strcpy(duppath, "index.html");
    else
        strcpy(duppath, path);

    sprintf(resourcepath, "%s/%s", conf.root, duppath);
    if (checkfile(resourcepath) == ERROR)
        status = FAILURE;

cleanup:
    if (resourcepath)
        free(resourcepath);
    if (duppath)
        free(duppath);
    return status;
}

void checkstartline(req_t *req)
{
    if (req->method == HTTP_METHOD_UNSUPPORTED)
        req->status = HTTP_NOTALLOWED;
    else if (checkpath(req->path) != SUCCESS)
        req->status = HTTP_BADREQUEST;
    else if (strcmp(req->version, HTTP_VERSION) != SUCCESS)
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
            return FATAL;

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
    size_t n = 0;
    char buffer[MSGSIZE];
    int status = 0;
    memset(buffer, '\0', MSGSIZE);

    // Read the start line
    nread = readline(fd, buffer);
    if (nread == FATAL)
    {
        req->status = HTTP_INTERNAL;
        return FATAL;
    }

    status = parsestartline(buffer, req);
    if (req->status != HTTP_SUCCESS)
        return status;

    checkstartline(req);
    if (req->status != HTTP_SUCCESS)
        return status;

    for (; req->status == HTTP_SUCCESS;)
    {
        nread = readline(fd, buffer);

        if (nread == -1)
        {
            perror("readline");
            req->status = HTTP_INTERNAL;
            return -1;
        }

        n += (size_t)nread;
        if (endofmsg(buffer, (size_t)nread))
            break;

        lower(buffer);

        status = parseheaders(buffer, req);
        if (status == ERROR)
            req->status = HTTP_BADREQUEST;
        else if (status == FAILURE)
            req->status = HTTP_ENTITIYTOOLARGE;
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

const char *req_strstatus(int status_code)
{
    switch (status_code)
    {
    case HTTP_SUCCESS:
        return "OK";
    case HTTP_CREATED:
        return "Created";
    case HTTP_BADREQUEST:
        return "Bad Request";
    case HTTP_UNAUTHORIZED:
        return "Unauthorized";
    case HTTP_FORBIDDEN:
        return "Forbidden";
    case HTTP_NOTFOUND:
        return "Not Found";
    case HTTP_NOTALLOWED:
        return "Method Not Allowed";
    case HTTP_TIMEOUT:
        return "Request Timeout";
    case HTTP_ENTITIYTOOLARGE:
        return "Request Entity Too Large";
    case HTTP_INTERNAL:
        return "Internal Server Error";
    case HTTP_NOTIMPL:
        return "Not Implemented";
    case HTTP_UNSUPPORTED:
        return "HTTP Version Not Supported";
    default:
        return "Bad Request";
    }
}

void req_send_error(int fd, int status_code)
{
    char buffer[MSGSIZE];
    memset(buffer, '\0', MSGSIZE);

    snprintf(buffer, MSGSIZE, "HTTP/1.1 %d %s\r\n", status_code, req_strstatus(status_code));
    send(fd, buffer, strlen(buffer), 0);
    snprintf(buffer, MSGSIZE, "Content-Length: %ld\r\n", strlen(req_strstatus(status_code)));
    send(fd, buffer, strlen(buffer), 0);
    snprintf(buffer, MSGSIZE, "Content-Type: text/plain\r\n");
    send(fd, buffer, strlen(buffer), 0);
    snprintf(buffer, MSGSIZE, "Connection: close\r\n");
    send(fd, buffer, strlen(buffer), 0);
    snprintf(buffer, MSGSIZE, "\r\n");
    send(fd, buffer, strlen(buffer), 0);
    snprintf(buffer, MSGSIZE, "%s", req_strstatus(status_code));
    send(fd, buffer, strlen(buffer), 0);
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

    if (nread == FATAL)
    {
        perror("recv");
        status = -1;
        goto safe_exit;
    }

#ifdef DEBUG
    hashmap_iterate(hashmap_begin(req->headers), NULL, "http.headers");
    fprintf(stdout, "%d %s %s\n", req->method, req->path, req->version);
    fprintf(stdout, "IP: %s\n\n", req->ip);
    fprintf(stdout, "Status code: %d\n", req->status);

#endif

    if (nread == FAILURE)
        goto send_message;

send_message:
    if (req->status != HTTP_SUCCESS)
        req_send_error(fd, req->status);

safe_exit:
    if (req != NULL)
        reqfree(req);
    close(fd);
    return status;
}
