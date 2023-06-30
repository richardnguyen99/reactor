cmake_minimum_required(VERSION 3.5...3.18)
project(jaws VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing(true)

add_executable(jaws main.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
