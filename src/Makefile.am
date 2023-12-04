lib_LTLIBRARIES = librx.la
librx_la_SOURCES =      \
    rx_connection.c     \
    rx_core.c           \
    rx_file.c           \
    rx_log.c            \
    rx_qlist.c          \
    rx_request.c        \
    rx_response.c       \
    rx_ring.c          \
    rx_route.c         \
    rx_string.c        \
    rx_thread.c        \
    rx_view.c         

librx_la_CFLAGS = \
    -I$(top_srcdir)/include \
    -Wall -Wextra -Werror -pedantic -std=c11 -fPIC -O3 

# if BUILD_DEV
# lib_LTLIBRARIES += librx-dev.la
# librx_dev_la_SOURCES = src/*.c
# librx_dev_la_CPPFLAGS = -I$(top_srcdir)/include
# librx_dev_la_CFLAGS = -g -O0 -DRX_DEBUG=1
# endif
