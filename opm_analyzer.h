#pragma once

#include "chip_analyzer.h"
#include "opm_voice_collector.h"

struct opm_analyzer {
	struct chip_analyzer chip_analyzer;

	int clock;
	uint8_t regs[256];
	uint8_t pmd;

	struct opm_voice_collector collector;
};

struct opm_analyzer * opm_analyzer_new(int clock);
void opm_analyzer_init(struct opm_analyzer *analyzer, int clock);
