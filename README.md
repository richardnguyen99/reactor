# Reactor

## Introduction

`reactor` is a simple web server written in C. The web server follows the
[Reactor Pattern][^2] and tries to solve the [C10k Problem][^1]. It is also
inspired by the implementation of NodeJS and the [libuv][^3] library.

## Build

### Prerequisites

I develop this projecton Fedora 38. It should work on other Linux distributions
as long as you have these following packages installed:

- `gcc`: GNU Compiler Collection &mdash; <https://gcc.gnu.org/>
- `cmake`: Cross-platform build system &mdash; <https://cmake.org/>
- `make`: GNU Make &mdash; <https://www.gnu.org/software/make/>
- `json-c`: JSON library in C &mdash; <https://github.com/json-c/json-c>

### Build from CMake

To build the project, run the following commands:

```bash



```

## Implementation

The web server contains a main event loop in a single thread. The event loop
is responsible for accepting new connections and dispatching them. If there is
a IO request, or a new data is available on a socket, the event loop will
dispatch the request to a worker thread.

All worker threads are managed by a thread pool. The thread pool will notify if
there is a job available. The worker thread will then process the request and
send the response back to the client.

### Event Loop

The main thread employs an [I/O event notification facility][^4] using `epoll(7)`
to monitor the file descriptors. The event loop will wait for events to occur
on the file descriptors. Here is the list of events that the event loop will
wait for:

- New connections
- New data available on a socket
- Timeout
- Signal
- Close connection

If there is a new connection, the event loop will add the file descriptor of the
new connection to the event queue.

### Thread Pool

In the beginning, a thread pool is created with a fixed number of threads. They
will wait for a job to be available in a queue.

If there is some data available on a socket, the event loop will dispatch them
to the task queue. The worker thread will then process the request and send the
appropriate response back to the client.

### Synchronization

The event loop and the worker threads share the same task queue. The task queue
is a simple FIFO queue. The event loop will add new tasks to the queue and the
worker threads will remove them from the queue.

The queue has a fixed size that is determined when the server is started. This
size will be defined by the user under the configuration file.

If the queue is full, the event loop needs to wait for an empty slot in the
queue. If the queu is empty, the worker threads will wait for a new task to be
added to the queue by the event loop. This techinque follows the [Producer-
Consumer pattern][^5], or a _bounded-buffer problem_.

Since the queue can be accessed by multiple threads, it needs to be protected so
that there is no [race condition][^6]. The protection uses a [mutex lock][^7] to
grant access to the queue to only one thread at a time.

The queue is basically a list of file descriptors that requests data. For
simplicity, the queue uses a ring buffer so that it remains the same size
without worrying about the memory allocation.

### HTTP Server

Although the main focus of this project is to implement a server that can handle
multiple connections, it provides simple HTTP endpoints to visualize what the
server can do.

The server provides the following endpoints:

- `/`: The root endpoint. It returns the index page.
- `/about`: It returns the about page.
- `/login`: It returns the login page. It also supports the `POST` method to
  login.
- `/static/*`: It returns the static files such as HTML, CSS and JavaScript
  files

Files sent to the client are read from the disk using [`mmap(2)`][^8]. Since
static files can be large, it is better to use `mmap` to read the files from the
disk.

## Change log

A list of changes can be found in the [CHANGELOG.md](CHANGELOG.md) file.

## Footnotes

[^1]: ()[https://en.wikipedia.org/wiki/C10k_problem]
[^2]: ()[https://en.wikipedia.org/wiki/Reactor_pattern]
[^3]: ()[https://libuv.org/]
[^4]: ()[https://man7.org/linux/man-pages/man7/epoll.7.html]
[^5]: ()[https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem]
[^6]: ()[https://en.wikipedia.org/wiki/Race_condition#In_software]
[^7]: ()[https://en.wikipedia.org/wiki/Mutual_exclusion]
[^8]: ()[https://man7.org/linux/man-pages/man2/mmap.2.html]
