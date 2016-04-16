CC=gcc
CFLAGS=-g -I.

sources = $(wildcard *.c)
objects = $(sources:.c=.o)

transducer: $(objects)
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -f $(objects) transducer
