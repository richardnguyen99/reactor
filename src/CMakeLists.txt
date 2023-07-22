# Configure CMAKE
cmake_minimum_required(VERSION 3.5...3.18)
project(reactor VERSION 0.1.0 LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    _reactor.c
    reactor.c
    http.c
    util.c
    dict.c
    poll.c
    _request.c
    request.c
    response.c
    ring_buffer.c
)

add_library(reactor_source ${SOURCES})# Cmake basic configuration

