ACLOCAL_AMFLAGS =-I m4

SUBDIRS = src lib test

bin_PROGRAMS = reactor
reactor_SOURCES = rx_main.c
reactor_CFLAGS = -Iinclude -Wall -Werror -Wextra -pedantic -std=c99 -O3
reactor_LDFLAGS = -Iinclude -Wall -Werror -Wextra -pedantic -std=c99 -O3
reactor_LDADD = src/librx.la -lpthread

test: test/rx_test
	./test/rx_test

.PHONY: test
