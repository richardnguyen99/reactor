cmake_minimum_required(VERSION 3.5...3.18)
project(jaws VERSION 0.1.0 LANGUAGES C)

include(CTest)
include(CheckLibraryExists)
include(CheckSymbolExists)

enable_testing(true)

# Uncomment this command to turn off debugging message
option(USE_DEBUG_FLAG "Enable Debug flag" OFF)
if (USE_DEBUG_FLAG)
    add_compile_options(-DDEBUG)
endif()
add_compile_options(-Wall -Werror -Wextra -Wpedantic)

# Copy env file from source dir to build dir
configure_file(${CMAKE_SOURCE_DIR}/.env ${CMAKE_BINARY_DIR}/.env COPYONLY)

# Check if pthread exists
check_library_exists(pthread pthread_create "" HAVE_PTHREAD)

# Check if 
check_symbol_exists(socket sys/socket.h HAVE_SOCKET)
check_symbol_exists(bind sys/socket.h HAVE_BIND)

if (NOT HAVE_SOCKET OR NOT HAVE_BIND)
    message(FATAL_ERROR "Required socket symbols not found. Cannot continue.")
endif()

# List source files to build the project
set(SOURCE_FILES
    main.c
    hashmap.c
    util.c
    socket.c
    http.c
)

# Build jaws executable based on these source files
add_executable(jaws ${SOURCE_FILES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Link the pthread library
if (HAVE_PTHREAD)
    target_link_libraries(jaws PRIVATE pthread)
endif()
