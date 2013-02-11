CC ?= cc
CFLAGS ?= -Os -pedantic -pipe -std=c99 -Wall

.PHONY: all clean strip

all: deldir strip

deldir: deldir.c
	$(CC) $(CFLAGS) -o deldir deldir.c

strip: .stripped

.stripped: deldir
	strip -s deldir
	touch .stripped

clean:
	rm -f deldir .stripped

