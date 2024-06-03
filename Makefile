CC=gcc
CFLAGS=-Wall -ggdb
LDFLAGS=-lz -lm
VGMDIR=

.PHONY: all extract

all: vgm2opm

vgm2opm: vgm2opm.o cmdline.o tools.o libvgm2x.a vgm/libvgminterpreter.a libfmvoice/libfmvoice.a
	$(CC) $^ -o $@ $(LDFLAGS)
vgm/libvgminterpreter.a: vgm/commands.o vgm/error.o vgm/header.o vgm/interpreter.o
	ar cr $@ $^
libvgm2x.a: chip_analyzer.o opn_analyzer.o opn_voice_collector.o opm_analyzer.o opm_voice_collector.o vgm_analyzer.o
	ar cr $@ $^
libfmvoice/libfmvoice.a:
	cd libfmvoice && make libfmvoice.a

%.o: %.c
	$(CC) -MMD -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o vgm/*.o *.d vgm/*.d vgm/*.a vgm2opm *.a
	cd libfmvoice && make clean

extract: vgm2opm
	$(if $(VGMDIR),,$(error Please set VGMDIR for extraction))
	$(if $(shell which unzip),,$(error "No unzip in $(PATH), consider doing apt-get install unzip"))
	mkdir -p /tmp/vgmzip
	rm -f /tmp/vgmzip/*
	find "$$VGMDIR" -name '*.zip' | while read zipname; do \
		echo "$$zipname"; \
		unzip -q -o -d /tmp/vgmzip "$$zipname"; \
		./vgm2opm -o `dirname "$$zipname"`/`basename "$$zipname" .zip`.opm /tmp/vgmzip/*.vg[mz]; \
		rm -f /tmp/vgmzip/*; \
	done
	rmdir /tmp/vgmzip
