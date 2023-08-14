/**
 * @file rx_socket.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Header for socket module that works with socket
 * @version 0.1
 * @date 2023-08-13
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _RX_SOCKET_H_
#define _RX_SOCKET_H_ 1

#include <rx_defs.h>
#include <rx_core.h>

#define rx_addr addrinfo

struct rx_socket
{
    int                fd;
    int                domain;
    int                type;
    int                protocol;
    int                blocking;

    struct sockaddr_in addr;
};

struct rx_socket *
rx_socket_create();

rx_status_t
rx_socket_prepare_listening(struct rx_socket *s);

rx_status_t
rx_socket_listen(struct rx_socket *s);

#endif /* _RX_SOCKET_H_ */
