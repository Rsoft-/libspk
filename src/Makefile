CC=gcc
INITIAL_CFLAGS=
CFLAGS=$(INITIAL_CFLAGS) -Wall
ARCH=$(shell $(CC) -dumpmachine)

all:
	mkdir -p ../lib/$(ARCH)
	$(CC) $(CFLAGS) -o ../lib/$(ARCH)/libspk.o -c spk.c
