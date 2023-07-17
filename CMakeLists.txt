# Cmake basic configuration
cmake_minimum_required(VERSION 3.5.0...3.19)
project(reactor VERSION 0.1.0 LANGUAGES C)

# Cmake modules for checking libraries and packages
include(CTest)
include(CheckIncludeFile)
enable_testing()

# Cmake set up for the project
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Check for required libraries

# Check for pthread
check_include_file(pthread.h HAVE_PTHREAD_H)
if(NOT HAVE_PTHREAD_H)
    message(FATAL_ERROR "pthread.h not found")
endif()

# Check for sys/epoll.h
check_include_file(sys/epoll.h HAVE_EPOLL_H)
if(NOT HAVE_EPOLL_H)
    message(FATAL_ERROR "sys/epoll.h not found")
endif()

# Check for linux sockets
check_include_file(sys/socket.h HAVE_SOCKET_H)
if(NOT HAVE_SOCKET_H)
    message(FATAL_ERROR "sys/socket.h not found")
endif()

# Set the C flags
set(CMAKE_C_FLAGS "")

# Check if CMAKE build type is debug
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
    add_compile_options(
        -g3
        -O0
    )
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(
        -O3
        -Wall 
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

# Add flags to compiler
add_compile_options(${CMAKE_C_FLAGS})

# Add debug macros 
add_definitions(-D_GNU_SOURCE)              # For GNU and Linux extensions
add_definitions(-D_POSIX_C_SOURCE=200809L)  # For POSIX extensions

check_include_file(limits.h HAVE_LIMITS_H)
if (HAVE_LIMITS_H)
    add_definitions(-DHAVE_LIMITS_H)
endif()

# Add the include directory
include_directories(include)

# Build the project
add_executable(reactor main.c)


# Add the source directory
add_subdirectory(src)

# Link the libraries
target_link_libraries(reactor PRIVATE reactor_source pthread)




