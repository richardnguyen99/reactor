# Frequently Asked Questions

This Markdown documentation is to answer some common questions and issues that
I have encountered while working on this project, and I hope this will give you
some explanation on what is going on.

## Common terminologies

### What is the __C10K__ problem?

The [Concurrent 10,000 Connections](http://www.kegel.com/c10k.html) problem, or
the __C10K__ problem, is a problem that requires developers to create a server
that can handle a large number of concurrent connections (10,000) with a limited
resource. This problem is coined by Dan Kegel in 1999.

### What is the __Reactor__ pattern?

The [Reactor pattern](https://en.wikipedia.org/wiki/Reactor_pattern) is one of
the patterns whose attempt is to solve the __C10K__ problem. It contains an
event loop that monitors file descriptors. If there is any event, either read or
write, from any file descriptor, the event loop will handle that.

### What is an event loop?

A typical server will have a server socket that listens for new connections and
a list of client sockets that are connected to the server. An event can be
anything between the client and the server. It could be a new connection, a
write event, a read event or a timeout event. However, these operations are
mostly blocking, which means that the server will wait for the event to be
finished. Other events from other clients will be _blocked_.

The event loop will monitor all the events and handles them accordingly. More
importantly, the event loop makes sure that all the events are non-blocking.
Every client will have chance to get its event handled.

### What is blocking/non-blocking I/O?

Blocking I/O is a type of I/O that will block the execution of the program until
the I/O operation is finished. For example, if a program needs to read a file
from the disk, it will wait until the file is read from the disk. This is
blocking I/O.

As a contrast, non-blocking operations don't block the execution of the program.
It returns immediately so that the program can continue to do other things. If
the operation will block, it will return an indicator for the program to try
again. Developers need to handle this error to make sure the operation returns
the whole data.

### What is a thread pool?

A thread pool is a list of threads idling and waiting for some tasks. When the
program is booting up, a thread pool will be created with a fixed number of
threads. These threads will wait for a task to be added to the task queue. The
task queue gets its tasks from the event loop and lets the worker threads take.

The reason why we need a thread pool instead of spawning a new thread for each
connection is that spawning a new thread is computationally expensive. Creating,
switching and destroying a thread can cause a lot of overhead. A thread poll
creates all the threads in the beginning and reuses them for other tasks.

### What is Producer-Consumer problem?

The [Producer-Consumer problem](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem)
is a classic synchronization problem in which there is a producer that produces
some works and a list of consumers that consume the works. The works are stored
in a buffer (sometimes the problem is called _bounded-buffer problem_). The main
thread (producer) produces some tasks and puts them into the buffer (the task
queue). The worker threads (consumers) take the tasks from the buffer and
process them.

## Development

## Design

## Implementation
