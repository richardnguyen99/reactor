#include "server.h"
#include "util.h"

void _load_from_args(struct __reactor_config *config, int argc, char *argv[]);
void _load_from_env(struct __reactor_config *config);
void _usage(const char *msg);

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
    memset(server->config.rootdir, '\0', PATH_MAX);
    memset(&(server->addr), 0, sizeof(struct sockaddr_in));

    server->events = NULL;

    return;
}

void server_load_config(server_t *server, int argc, char *argv[]) 
{
    debug("Loading configuration...\n\n");

    if (argc > 1)
        _load_from_args(&(server->config), argc, argv);

    return;
}

void server_boot(server_t *server) 
{
    debug("Booting up the server...\n\n");

    debug("Server is ready...\n\n");

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

void _load_from_args(struct __reactor_config *config, int argc ,char *argv[]) 
{
    debug("Loading configuration from command line arguments...\n\n");

    int opt = 0;
    static struct option long_options[] = 
    {
        {"port", optional_argument, NULL, 'p'},
        {"rootdir", optional_argument, NULL, 'r'},
        {"nthreads", optional_argument, NULL, 't'},
        {"config", optional_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'},
        {NULL, no_argument, NULL, 0}
    };


    // Parse option from command line arguments based on long options
    while ((opt = getopt_long(argc, argv, "p:r:t:c:h", long_options, NULL)) != -1)
    {
        switch (opt) 
        {
            case 'p':
                config->port = (uint16_t)atoi(optarg);

                break;

            case 'r':
                strncpy(config->rootdir, optarg, PATH_MAX);

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


void _load_from_env(struct __reactor_config *config)
{
    return;
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
