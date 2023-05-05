CC=gcc
CFLAGS=-Wall -ggdb
LDFLAGS=

.PHONY: all

all: vgm2opm

vgm2opm: vgm2opm.c chip_analyzer.o opn_analyzer.o ym2151_analyzer.o vgm/commands.o vgm/error.o vgm/header.o vgm/interpreter.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -MMD -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o vgm2opm
