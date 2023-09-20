CC := gcc
COMMON_FLAGS := -Werror -std=c99 -Iinclude
DEV_FLAGS := $(COMMON_FLAGS) -g -O0
PROD_FLAGS := $(COMMON_FLAGS) -Wall -Wextra -Wpedantic -O3
CFLAGS := $(PROD_FLAGS)

LIBSOURCES = $(wildcard src/*.c)

SRCDIR := src
INCDIR := include
OBJDIR := obj

SRCS 	:= $(wildcard src/*.c)
HDRS 	:= $(wildcard include/*.h)
OBJS 	:= $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEVOBJS := $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%-dev.o)


.PHONY: all clean

all: librx.a rx_main.o
	$(CC) -o reactor rx_main.o -L. -lrx -lpthread

librx.a: $(OBJS)
	ar rcs librx.a $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@if [ ! -d $(OBJDIR) ]; then \
		echo "Creating $(OBJDIR)"; \
		mkdir -p $(OBJDIR); \
	fi

	$(CC) $(CFLAGS) -c $< -o $@

rx_main.o: rx_main.c
	$(CC) $(CFLAGS) -c $^ -o $@

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
	$(CC) $(DEV_FLAGS) -c $^ -o $@

clean:
	rm -f obj/*.o rx_main*.o *.a

clean-all: clean
	rm -f reactor*
