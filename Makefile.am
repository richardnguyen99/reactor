ACLOCAL_AMFLAGS =-I m4

SUBDIRS = src

bin_PROGRAMS = reactor
reactor_SOURCES = rx_main.c
reactor_CFLAGS = -Iinclude -Wall -Werror -O3 -Wextra -pedantic -std=c99
reactor_LDADD = src/librx.la -lpthread

# if BUILD_TEST
# check_PROGRAMS = rx_test
# rx_test_SOURCES = test/rx_test.c
# rx_test_LDADD = librx.a -lpthread
# endif

