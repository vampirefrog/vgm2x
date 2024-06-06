#pragma once

#include "chip_analyzer.h"
#include "opl_voice_collector.h"

struct opl_analyzer {
	struct chip_analyzer chip_analyzer;

	int clock;
	uint8_t regs[512];

	struct opl_voice_collector collector;

	int ym_dac;
	int num_channels;
};

struct opl_analyzer *opl_analyzer_new(int clock, int num_channels);
void opl_analyzer_init(struct opl_analyzer *analyzer, int clock, int num_channels);
