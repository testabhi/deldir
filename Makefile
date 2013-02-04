CC ?= cc
CFLAGS ?= -Os -pedantic -pipe -std=c99 -Wall

.PHONY: all clean

all: deldir
deldir: deldir.c
	$(CC) $(CFLAGS) -o deldir deldir.c

clean:
	rm -f deldir

