CC=gcc
CFLAGS=-ggdb -Wall $(shell pkg-config alsa jack --cflags)
PROGS=push-reader command-parser

.PHONY: all

all: $(PROGS)

push-reader: push-reader.o ../push_reader.o ../error.o ../commands.o
command-parser: command-parser.o ../push_reader.o ../command_parser.o ../error.o ../commands.o

%.o: %.c
	$(CC) -MMD -c $< -o $@ $(CFLAGS)
clean:
	rm -f *.o *.d $(PROGS)
