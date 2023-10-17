bin_PROGRAMS = rx_test

rx_test_SOURCES = \
    rx_test_accept_encoding_header.c                                           \
    rx_test_add.c                                                              \
    rx_test_host_header.c                                                      \
    rx_test_method.c                                                           \
    rx_test_parse_header.c                                                     \
    rx_test_qlist.c                                                            \
    rx_test_ring.c                                                             \
    rx_test_subtract.c                                                         \
    rx_test_uri.c                                                              \
    rx_test_version.c                                                          \
    rx_test.c

rx_test_CFLAGS =                                                               \
    -I$(top_srcdir)/include                                                    \
    -I$(top_srcdir)/lib                                                        \
    -Wall -Wextra -Werror -Wpedantic

rx_test_LDFLAGS = \
    -L$(top_srcdir)/lib/unity \
    -L$(top_srcdir)/src \
    -L$(top_srcdir)

rx_test_LDADD = $(top_srcdir)/src/librx.la $(top_srcdir)/lib/unity/libunity.la -lpthread
