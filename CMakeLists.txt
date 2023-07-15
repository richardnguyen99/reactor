# Configure CMAKE
cmake_minimum_required(VERSION 3.5...3.18)
project(reactor VERSION 0.1.0 LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Check if linux limits.h is available
include(CheckIncludeFile)

check_include_file(limits.h HAVE_LIMITS_H)

if (HAVE_LIMITS_H)
    add_definitions(-DHAVE_LIMITS_H)
endif()

# Check if CMAKE_DEBUG_FLAG is set
option (CMAKE_DEBUG_FLAG "Set to ON to enable debug flags" ON)

# Set compiler flags according to CMAKE_DEBUG_FLAG
if (CMAKE_DEBUG_FLAG)
    add_compile_options(-g -O0 -DDEBUG)
else()
    add_compile_options(-Wall -Wextra -Werror)
    add_compile_options(-O3 -Wpendatic)
endif()



include_directories(include)

add_executable(reactor main.c)

add_subdirectory(src)

target_link_libraries(reactor PRIVATE reactor_source)
