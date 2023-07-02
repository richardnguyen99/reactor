cmake_minimum_required(VERSION 3.5...3.18)
project(jaws VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing(true)

# Uncomment this command to turn off debugging message
add_compile_options(-DDEBUG)

# List source files to build the project
set(SOURCE_FILES
    main.c
    util.c
    socket.c
)

# Build jaws executable based on these source files
add_executable(jaws ${SOURCE_FILES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Link the pthread library
target_link_libraries(jaws pthread)
