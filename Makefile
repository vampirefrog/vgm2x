CC=gcc
CFLAGS=-Wall -ggdb
LDFLAGS=-lz -lm
VGMDIR=

.PHONY: all extract

all: vgm2opm

vgm2opm: vgm2opm.o cmdline.o tools.o chip_analyzer.o opn_analyzer.o opn_voice_collector.o opm_analyzer.o opm_voice_collector.o vgm/commands.o vgm/error.o vgm/header.o vgm/interpreter.o libfmvoice/libfmvoice.a
	$(CC) $^ -o $@ $(LDFLAGS)
libfmvoice/libfmvoice.a:
	cd libfmvoice && make libfmvoice.a

%.o: %.c
	$(CC) -MMD -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o *.d vgm2opm
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
