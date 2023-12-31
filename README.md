# Reactor

[Screencast.webm](https://github.com/richardnguyen99/reactor/assets/58826215/632d2af5-f029-417b-b8e5-aed12e0fca0a)

Reactor is an HTTP server written in pure C:

- [x] **Experimental** and **Educational** purpose only
- [x] Attempt to be compliant with HTTP/1.1
- [x] Support `GET`, `POST`, and `HEAD` method
- [x] Support routing
- [x] Support static files
- [x] Support template rendering
- [x] Support eror handling

Limitations:

- [ ] No TLS support
- [ ] Miss request timeout feature
- [ ] Miss keep-alive feature
- [ ] Dockerfile to support cross-platform build

## File structure

As the nature of C project, there are a lot of files and folders that associates
with the development process. However, for the core components of the server,
which is the HTTP server part, the following files and folders are the most
important:

```sh
.   (root directory)
├── configure                                 # configure script     (generated)
├── configure.ac                              # autoconf template
├── README.md
├── Makefile.am                               # Automake template
├── rx_main.c                                 # main program
├── lib
│   └── unity                                 # source for unity testing
├── include                                   # header files
├── m4/                                       # autoconf macros
├── pages                                     # HTML pages
├── public                                    # static files
├── src                                       # source files
└── test                                      # unit tests
```

The core program is in `rx_main.c`, which has the `main()` function. All the
headers (`*.h`) are in the `include` folder. The source files (`*.c`) are in the
`src` folder. The unit tests are in the `test` folder.

All C source files include two core headers:

- `rx_config.h`: Contain all the necessary standard C and Linux headers.
- `rx_core.h`: Contain all the core HTTP, connection and thread headers.

In order to access all core structures and public functions through these two
headers, `rx_core.h` declares all the structures but the definitions fall into
the corresponding header files. For example:

```c
// rx_core.h
struct rx_response;

#include <rx_response.h>

// rx_response.h
struct rx_response {
  // ...
};
```

More details about the file structure can be found in the `include/rx_core.h`.

## Description

> This is an attempt to solve the C10K problem, and it's solely for educational
> purpose.

The server maintains two core components

- An _event loop_ that monitors the file descriptors that represent the
  connection between the server and the clients.
- A _thread pool_ that handles the requests from the clients.

### Event loop

In `rx_main.c`, the server creates an event loop using `epoll(7)` as the core
mechanism to monitor the file descriptors.

A simple explanation of the event loop is as follows:

- The event loop waits for the events from the registered file descriptors.
- When an event occurs, the event loop will handle the event by dispatching it
  to the corresponding handler.
- There are 4 types of events:
  - `EPOLLIN`: The client sends a request to the server for processing.
  - `EPOLLOUT`: The server sends a response to the client.
  - `EPOLLERR`: An error occurs (can be either the client or the server).
  - `server_fd`: A new client connects to the server.

A important note is that the event loop is **single-threaded**. It means that
the event loop can only handle one event at a time. Therefore, the event loop
should not do any heavy work. Instead, it should dispatch the event to the
thread pool for processing.

To make sure that the event loop can handle the events as fast as possible, the
file descriptors (socket) are set to non-blocking mode. It means that system
calls like `read(2)` and `write(2)` will return immediately. However, this might
lead to the problem of _partial read_ and _partial write_.

In order to solve the problem of _partial read_ and _partial write_, each
connection maintains two buffers, one for request and one for response.

- When the event loop receives an `EPOLLIN` event from a file descriptor, the
  connection tries to read and put the data into the request buffer. The event
  loop will keep reading until the `read(2)` system call returns `0` (no more
  data).
- After the request buffer is fully read, the connection will be passed to the
  thread pool for processing. After processing the request, the connection will
  construct a response message and put it into the response buffer.
- When the event loop receives an `EPOLLOUT` event from a file descriptor, the
  connection tries to write the data from the response buffer to the client. The
  event loop will keep writing until the `write(2)` system call returns `0` (no
  more data).
- After the response is fully sent, the connection will be closed.

### Thread pool

Instead of creating a new thread for each connection, the server maintains a
thread pool to handle requests and responses from a connection. A thread pool
essentially is a set of pre-created threads that are ready to handle requests.
The thread pool follows the _producer-consumer_ pattern, which maintains one
producer thread, multiple consumer threads and a shared queue.

- The producer thread is responsible for accepting new connections, reading
  requests into a buffer and putting it into the shared queue. This is done by
  the event loop in the main thread.
- The consumer threads are responsible for taking a connection from the shared
  queue, processing the request and constructing the response back to the
  client. After the response is fully constructed, the thread will notify the
  event loop to write the response back to the client, and get another
  connection.
- The shared queue is a bounded ring buffer that stores connections as tasks. As
  the queue is shared and accessed by multiple threads, it needs to be protected
  and synchronized. To achieve this, the queue uses a `pthread_mutex_t` lock and
  two `sem_t` semaphores.
  - The `pthread_mutex_t` lock is used to protect the queue from concurrent
    access. At any time, only one thread, either consumer or producer, can
    access the queue.
  - One `empty` semaphore is used by the consumers to signal the producer thread
    that the queue still has empty slots. When the queue is full, this semaphore
    will make the producer thread blocked.
  - One `full` semaphore is used by the producer thread to signal the consumer
    threads that the queue has at least one tasks. When the queue is empty, this
    semaphore will make the consumer threads blocked.

## Development

### Prerequisites

This project is developed and tested on Fedora 38. However, it should work on
other Linux distributions as well as long as the following dependencies are
installed:

- `gcc`
- `make`
- `epoll(7)`
- `pthread(7)`

### Build

1. Clone the project:

```sh
git clone https://github.com/richardnguyen99/reactor.git
```

2. Generate a `Makefile`:

```sh
./configure
```

3. Build the project:

The following script is used to generate and build the project:

```sh
make
./reactor
```

If you want to use the dev build, which is less strict and more verbose, you can
use the following script:

```sh
make dev
./reactor-dev
```

You can test the dev build with `valgrind` for memory leak detection:

```sh
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes  ./reactor-dev
```

### Test

The following script is used to run the test suite:

```sh
make test
```

## License

This project is under the MIT License. See the [LICENSE](LICENSE) file for the
full license text.

## References

- [HTTP/1.1](https://tools.ietf.org/html/rfc2616)
- [HTTP/1.1 Semantics and Content](https://tools.ietf.org/html/rfc7231)
- [HTTP/1.1 Message Syntax and Routing](https://tools.ietf.org/html/rfc7230)
- [HTTP/1.1 Conditional Requests](https://tools.ietf.org/html/rfc7232)
- [Linux epoll](https://man7.org/linux/man-pages/man7/epoll.7.html)
- [The C10K problem](http://www.kegel.com/c10k.html)
- [Producer-consumer problem](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem)
