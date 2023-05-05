#pragma once

#include "chip_analyzer.h"

struct opm_voice_operator {
	uint8_t dt1_mul;
	uint8_t tl;
	uint8_t ks_ar;
	uint8_t ame_d1r;
	uint8_t dt2_d2r;
	uint8_t d1l_rr;
};

struct opm_voice {
	uint8_t fb_connect;
	int vgm_ofs;
	uint8_t chan_used_mask;
	int note_usage[127];
	struct opm_voice_operator operators[4];
};

struct ym2151_analyzer {
	struct chip_analyzer chip_analyzer;

	int clock;
	uint8_t regs[256];

	struct opm_voice *voices;
	int num_voices;
};

struct ym2151_analyzer * ym2151_analyzer_new(int clock);
void ym2151_analyzer_init(struct ym2151_analyzer *analyzer, int clock);
void ym2151_analyzer_dump_voices(struct ym2151_analyzer *analyzer);
