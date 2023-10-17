ACLOCAL_AMFLAGS =-I m4

SUBDIRS = src lib test

bin_PROGRAMS = reactor
reactor_SOURCES = rx_main.c
reactor_CFLAGS = -Iinclude -Wall -Werror -Wextra -pedantic -std=c99 -O3
reactor_LDFLAGS = -Iinclude -Wall -Werror -Wextra -pedantic -std=c99 -O3
reactor_LDADD = src/librx.la -lpthread


test: test/rx_test
	./test/rx_test

dev: 
	gcc -Werror -g -O0 -Iinclude -DRX_DEBUG=1 									\
	src/rx_connection.c 														\
	src/rx_core.c 																\
	src/rx_file.c 																\
	src/rx_log.c 																\
	src/rx_qlist.c																\
	src/rx_request.c 															\
	src/rx_response.c 															\
	src/rx_ring.c 																\
	src/rx_route.c 																\
	src/rx_string.c 															\
	src/rx_thread.c 															\
	src/rx_view.c 																\
	rx_main.c -o reactor-dev -lpthread


.PHONY: test dev
