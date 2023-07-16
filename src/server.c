#include "server.h"
#include "util.h"
#include "poll.h"

void _set_args(struct __reactor_config *config, int argc, char *argv[]);
void _load_from_env(struct __reactor_config *config);
void _usage(const char *msg);

void _prepare_socket(struct reactor_server *server);
void _set_nonblocking(int fd);

// ============================================================================

void server_print(server_t *server) 
{
#ifdef DEBUG
    printf("Hello, world!\n");
#endif

    return;
}

void server_init(struct reactor_server *server)  
{
    debug("Initializing server instance...\n\n");

    // Initialize the server instance
    memset(server, 0, sizeof(struct reactor_server));
    memset(&(server->config), 0, sizeof(struct __reactor_config));
    memset(server->config.config_path, '\0', PATH_MAX   );
    memset(server->config.root_dir, '\0', PATH_MAX);
    memset(&(server->config.port), '\0', sizeof(port_t));
    memset(&(server->addr), 0, sizeof(struct sockaddr_in));

    server->config.port.number = 0;
    memset(&(server->config.port.str), '\0', 6);

    server->config.nthreads = 0;
    server->config.root_dir[0] = '\0';

    memset(&server->events, '\0', sizeof(struct epoll_event) * MAX_EVENTS);

    return;
}

void server_load_config(server_t *server, int argc, char *argv[]) 
{
    debug("Loading configuration...\n\n");

    if (argc > 1)
        _set_args(&(server->config), argc, argv);
    
    _load_from_env(&(server->config));

    

    debug("Configuration loaded...\n");
    debug("Port: %d\n", server->config.port.number);
    debug("Root directory: %s\n", server->config.root_dir);
    debug("Number of threads: %ld\n", server->config.nthreads);

    return;
}

void server_boot(server_t *server) 
{
    debug("Booting up the server...\n\n");

    // Prepare the socket for listening server
    _prepare_socket(server);

    if (listen(server->sockfd, SOMAXCONN) == -1)
        DIE("(boot) listen");

    // Set non blocking socket for epoll
    _set_nonblocking(server->sockfd);
    
    // Create an epoll instance
    server->epollfd = poll_create(0);
    
    if (server->epollfd == -1)
        DIE("(boot) poll_create");

    // Add the listening socket to the epoll instance
    if (poll_add(server->epollfd, server->sockfd, (EPOLLIN)) == -1)
        DIE("(boot) poll_add");

    server->queue = queue_init(server->config.queue_cap);

    if (server->queue == NULL)
        DIE("(boot) queue_init");

    server->tpool = tp_create(server->config.queue_cap, server->config.nthreads);

    if (server->tpool == NULL)
        DIE("(boot) tp_create");

    debug("Server is ready...\n\n");
    debug("Server address: %s\n", inet_ntoa(server->addr.sin_addr));
    debug("Server port: %d\n", ntohs(server->addr.sin_port));

    return;
}

void server_route(server_t *server, const char *path, void (*handler)(void)) 
{
    debug("Routing the server...\n\n");

    return;
}

void server_start(server_t *server) 
{
    debug("Starting the server...\n\n");

    return;
}

void server_usage()
{
    _usage(no_error_msg);
}

// ============================================================================

void _set_args(struct __reactor_config *config, int argc ,char *argv[]) 
{
    debug("Loading configuration from command line arguments...\n\n");

    int opt = 0;
    static struct option long_options[] = 
    {
        {"port", required_argument, NULL, 'p'},
        {"queue", required_argument, NULL, 'q' },
        {"rootdir", required_argument, NULL, 'r'},
        {"nthreads", required_argument, NULL, 't'},
        {"config", required_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'},
        {NULL, no_argument, NULL, 0}
    };

    // Parse option from command line arguments based on long options
    while ((opt = getopt_long(argc, argv, "p:r:t:c:h", long_options, NULL)) != -1)
    {
        switch (opt) 
        {
            case 'p':
                debug("port %s\n", optarg);
                config->port.number = (uint16_t)atoi(optarg);
                strncpy(config->port.str, optarg, PORTSIZE);

                break;

            case 'q':
                config->queue_cap = (size_t)atoi(optarg);

                break;

            case 'r':
                strncpy(config->root_dir, optarg, PATH_MAX);

                break;

            case 't':
                config->nthreads = (size_t)atoi(optarg);

                break;

            case 'c':
                strncpy(config->config_path, optarg, PATH_MAX);

                break;

            case 'h':
                _usage(no_error_msg);

                break;
            default:
                char msg[80];
                sprintf(msg, "Unknown option: %c\n", optopt);
                _usage(msg);

                break;
        }
    }

    return;
}

int _parse_env_to_config(int fd, char *buf, struct __reactor_config *config)
{
    char *key, *value;
    const char *delim = "=";

    key = strtok(buf, delim);
    value = strtok(NULL, delim);

    if (key == NULL || value == NULL)
        return ERROR;

    if (strcmp(key, "PORT") == 0)
    {
        config->port.number = (uint16_t)atoi(value);
        strncpy(config->port.str, value, PORTSIZE);
    }

    if (strcmp(key, "QUEUE") == 0)
        config->queue_cap = (size_t)atoi(value);
    
    if (strcmp(key, "ROOTDIR") == 0)
        strncpy(config->root_dir, value, PATH_MAX);

    if (strcmp(key, "NTHREADS") == 0)
        config->nthreads = (size_t)atoi(value);

    return SUCCESS;
}

int _read_env_to_config(int fd, struct __reactor_config *config)
{
    char buf[BUFSIZ - 1];

    for (;;)
    {
        ssize_t nread = read_line(fd, buf, BUFSIZ);

        if (nread == -1)
            return ERROR;

        if (nread == 0)
            break;

        if (_parse_env_to_config(fd, buf, config) == -1)
            DIEASYOUWISH("Invalid configuration file.\n");
    }

    return SUCCESS;
}

void _load_from_env(struct __reactor_config *config)
{
    // If the configuration file is loaded
    if (config->config_path[0] != '\0')
    {
        int fd = open(config->config_path, O_RDONLY);

        if (fd == -1)
            DIE("(load_from_env) open");

        if (_read_env_to_config(fd, config) == -1)
            DIE("(load_from_env) read");

        if (close(fd) == -1)
            DIE("(load_from_env) close");

        return;
    }

    if (config->nthreads <= 0)
        config->nthreads = 4;

    if (config->queue_cap <= 0)
        config->queue_cap = 20;

    if (config->port.number < 1024 || config->port.number > 49151)
    {
        config->port.number = 7777;
        strncpy(config->port.str, "7777", PORTSIZE);
    }

    if (config->root_dir[0] == '\0')
        strncpy(config->root_dir, "public", PATH_MAX);
}

void _load_addr_info(struct sockaddr_in * addr, struct addrinfo *p)
{
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

    addr->sin_addr = ipv4->sin_addr;
    addr->sin_family = p->ai_family;
    addr->sin_port = ipv4->sin_port;
}

void _prepare_socket(struct reactor_server *server)
{
    struct addrinfo hints, *res, *p;
    int status, sockfd, yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = AI_PASSIVE;        // Fill in my IP for me

    // Get address info based on hints and server configuration
    if ((status = getaddrinfo(NULL, server->config.port.str, &hints, &res)) != 0)
        DIE("(prepare_socket) getaddrinfo");

    for (p = res; p != NULL; p = p->ai_next)
    {
        // Create a socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        // Set socket options
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
            DIE("(prepare_socket) setsockopt");

        // Bind the socket to the address
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL)
        DIE("Failed to bind socket");

    _load_addr_info(&server->addr, p);
    server->sockfd = sockfd;

    freeaddrinfo(res);
}

void _set_nonblocking(int fd)
{
    int status = fcntl(fd, F_GETFL, 0);

    if (status == -1)
        DIE("(set_nonblocking) fcntl");

    status |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, status) == -1)
        DIE("(set_nonblocking) fcntl");
}

void _usage(const char *msg)
{
    int status = msg == NULL;
    FILE *stream = status ? stdout : stderr;

    fprintf(stream, ""                                                          \
"Reactor Web Server v%d.%d.%d\n"                                                \
"An experimental and education-only web server written in C.\n"                 \
"\n"                                                                            \
"Usage: reactor [OPTION]...\n"                                                  \
"\n"                                                                            \
"A list of short and long options for the server:\n"                            \
"  -p, --port=PORT              Port number to listen on\n"                     \
"  -r, --rootdir=ROOTDIR        Root directory of the server\n"                 \
"  -t, --nthreads=NTHREADS      Number of threads for the thread pool\n"        \
"  -c, --config=CONFIG_PATH     Path to the configuration file.\n"              \
"                               This option will override all other options.\n" \
"\n"                                                                            \
"  -h, --help                   Display this help and exit\n"                   \
"", REACTOR_VERSION_MAJOR, REACTOR_VERSION_MINOR, REACTOR_VERSION_PATCH);
    
    exit(status);
}
