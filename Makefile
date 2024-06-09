CC=gcc
CFLAGS=-Wall -ggdb $(shell pkg-config --cflags libarchive libzip)
LDFLAGS=-lz -lm $(shell pkg-config --libs libarchive libzip)
VGMDIR=

.PHONY: all extract

all: vgm2x

vgm2x: vgm2x.o cmdline.o tools.o libvgm2x.a vgm/libvgminterpreter.a midilib/libmidi.a libfmvoice/libfmvoice.a libvgm/utils/DataLoader.o libvgm/utils/FileLoader.o libvgm/utils/MemoryLoader.o
	$(CC) $^ -o $@ $(LDFLAGS)
vgm/libvgminterpreter.a: vgm/commands.o vgm/error.o vgm/header.o vgm/interpreter.o
	ar cr $@ $^
libvgm2x.a: chip_analyzer.o opl_analyzer.o opl_voice_collector.o opm_analyzer.o opm_voice_collector.o opn_analyzer.o opn_voice_collector.o vgm_analyzer.o
	ar cr $@ $^
midilib/libmidi.a:
	cd midilib && make libmidi.a
libfmvoice/libfmvoice.a:
	cd libfmvoice && make libfmvoice.a

%.o: %.c
	$(CC) -MMD -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o vgm/*.o *.d vgm/*.d vgm/*.a vgm2x *.a
	cd libfmvoice && make clean
	cd midilib && make clean
