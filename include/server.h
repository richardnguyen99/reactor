/**
 * @file server.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Server header file
 * @version 0.1.2
 * @date 2023-07-14
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_SERVER_H
#define _REACTOR_SERVER_H 1

#include "defs.h"
#include "util.h"
#include "thread_pool.h"

#define PORTSIZE 6
typedef struct {
    uint16_t number;
    char str[PORTSIZE];
} port_t;


/* This is a part of reactor_server struct. Do not use by its own */
struct __reactor_config {

    /* Number of threads for the thread pool */
    size_t nthreads;

    /* Capacity for task queue */
    size_t queue_cap;

    /* Root directory of the server */
    char root_dir[PATH_MAX];

    /* Path to the configuration file */
    char config_path[PATH_MAX];

    /* Port number to listen on */
    port_t port;
};

struct reactor_server 
{
    /* Listening socket file descriptor of the server instance  */
    int sockfd;

    /* File descriptor to an epoll instance used by the server instance  */
    int epollfd;
    
    /* Epoll events associated with a file descriptor for monitoring */
    struct epoll_event events[MAX_EVENTS];

    /* Address and info of the server instance */
    struct sockaddr_in addr;

    /* Thread pool for handling IO events */
    struct thread_pool *tpool;

    /* Ring buffer data structure for handling tasks */
    struct queue *queue;

    /* Contains the configuration of the server instance */
    struct __reactor_config config;
};

typedef struct reactor_server server_t;

/**
 * @brief Initialize a server instance with malloc'd memory
 * 
 * @param server A pointer to a server instance
 */
void server_init(struct reactor_server *server);

/**
 * @brief Load configuration based on environment and command line arguments
 * 
 * @param server  A pointer to a server instance
 * @param argc  The number of command line arguments from main
 * @param argv  The command line arguments from main
 */
void server_load_config(struct reactor_server *server, int argc, char *argv[]);

/**
 * @brief Boot up the server and prepare the socket for listening
 * 
 * @param server  A pointer to a server instance
 */
void server_boot(struct reactor_server *server);

/**
 * @brief Add a route and its handler to the server instance
 * 
 * @param server A pointer to server instance
 * @param path An endpoint path
 * @param handler A function pointer to the handler to be called when the 
 *                endpoint is hit
 */
void server_route(struct reactor_server *server, const char *path, 
                        void (*handler)(void));


/**
 * @brief Start the server and listen for incoming connections
 * 
 * @param server A pointer to server instance
 */
void server_start(struct reactor_server *server);

/**
 * @brief Print out server information when DEBUG flag is enabled
 * 
 * @param server A pointer to server instance
 */
void server_print(struct reactor_server *server);

#define no_error_msg NULL

/**
 * @brief Print out server usage. 
 * 
 */
void server_usage();

#endif // _REACTOR_SERVER_H
