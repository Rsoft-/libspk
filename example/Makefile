CC=gcc
INITIAL_CFLAGS=
CFLAGS=$(INITIAL_CFLAGS) -Wall
CFLAGS+=-I../include
ARCH=$(shell $(CC) -dumpmachine)

all:
	$(CC) $(CFLAGS) -o test-$(ARCH) test.c ../lib/$(ARCH)/libspk.o
