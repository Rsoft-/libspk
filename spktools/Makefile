CC=gcc
CFLAGS=-Wall

ARCH=$(shell $(CC) -dumpmachine)

.PHONY: all genspk spkx

all: genspk spkx

genspk:
	$(CC) $(CFLAGS) -o genspk genspk.c

spkx:
	$(CC) $(CFLAGS) -I../include -o spkx spkx.c ../lib/$(ARCH)/libspk.o
