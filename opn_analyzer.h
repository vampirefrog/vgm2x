#pragma once

#include "chip_analyzer.h"
#include "opn_voice_collector.h"

struct opn_analyzer {
	struct chip_analyzer chip_analyzer;

	int clock;
	uint8_t regs[512];

	struct opn_voice_collector collector;

	int ym_dac;
	int num_channels;
};

struct opn_analyzer *opn_analyzer_new(int clock, int num_channels);
void opn_analyzer_init(struct opn_analyzer *analyzer, int clock, int num_channels);
void opn_analyzer_dump_voices(struct opn_analyzer *analyzer);
