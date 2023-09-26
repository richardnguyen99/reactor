CC := gcc
COMMON_FLAGS := -Werror -std=c99 
DEV_FLAGS := $(COMMON_FLAGS) -g -O0 -DRX_DEBUG=1
PROD_FLAGS := $(COMMON_FLAGS) -Wall -Wextra -Wpedantic -O3
CFLAGS := $(PROD_FLAGS)

LIBSOURCES = $(wildcard src/*.c)

LIBDIR := lib
TSTDIR := test
SRCDIR := src
INCDIR := include
OBJDIR := obj

SRCS 	:= $(wildcard src/*.c)
TEST  	:= $(wildcard test/*.c)
LIBS 	:= $(wildcard lib/**/*.c)
HDRS 	:= $(wildcard include/*.h)
OBJS 	:= $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TSTS 	:= $(TEST:$(TSTDIR)/%.c=$(OBJDIR)/$(TSTDIR)/%.o)
DEVOBJS := $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%-dev.o)
LIBA 	:= $(LIBS:$(LIBDIR)/%.c=$(OBJDIR)/%.o)

LIBUNITYSRC := $(wildcard lib/unity/*.c)
LIBUNITYOBJ := $(LIBUNITYSRC:lib/unity/%.c=$(OBJDIR)/lib/unity/%.o)


.PHONY: all clean test

all: librx.a rx_main.o rx_test
	$(CC) -o reactor rx_main.o -L. -lrx -lpthread

test: libunity.a rx_test
	./rx_test

rx_test: libunity.a librx.a $(TSTS)
	$(CC) -o rx_test $(TSTS) -L. -lunity -lrx -lpthread

rx_test.o: test/rx_test.c
	$(CC) $(CFLAGS) -Iinclude -Ilib -c $< -o $@

$(OBJDIR)/test/%.o: test/%.c
	@if [ ! -d $(OBJDIR)/test ]; then \
		echo "Creating $(OBJDIR)/test"; \
		mkdir -p $(OBJDIR)/test; \
	fi

	$(CC) $(CFLAGS) -Iinclude -Ilib -c $< -o $@

libunity.a: $(LIBUNITYOBJ)
	@echo "Creating libunity.a"
	ar rcs libunity.a $^

$(OBJDIR)/lib/unity/%.o: lib/unity/%.c
	@echo "Compiling object files for libunity"
	@if [ ! -d $(OBJDIR)/lib/unity ]; then \
		echo "Creating $(OBJDIR)/lib/unity"; \
		mkdir -p $(OBJDIR)/lib/unity; \
	fi

	$(CC) $(CFLAGS) -Ilib/unity -c $< -o $@

librx.a: $(OBJS)
	ar rcs librx.a $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@if [ ! -d $(OBJDIR) ]; then \
		echo "Creating $(OBJDIR)"; \
		mkdir -p $(OBJDIR); \
	fi

	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

rx_main.o: rx_main.c
	$(CC) $(CFLAGS) -Iinclude -c $^ -o $@

dev: librx-dev.a rx_main-dev.o
	$(CC) -o reactor-dev rx_main-dev.o -L. -lrx-dev -lpthread

librx-dev.a: $(DEVOBJS)
	ar rcs librx-dev.a $^

$(OBJDIR)/%-dev.o: $(SRCDIR)/%.c
	@if [ ! -d $(OBJDIR) ]; then \
		echo "Creating $(OBJDIR)"; \
		mkdir -p $(OBJDIR); \
	fi

	$(CC) $(DEV_FLAGS) -c $< -o $@

rx_main-dev.o: rx_main.c
	$(CC) $(DEV_FLAGS) -Iinclude -c $^ -o $@

clean:
	rm -rf obj rx_main*.o *.a rx_test*

clean-all: clean
	rm -f reactor*
