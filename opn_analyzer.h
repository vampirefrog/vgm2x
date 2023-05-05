#pragma once

#include "chip_analyzer.h"

struct opn_voice_operator {
	uint8_t dt_mul;
	uint8_t tl;
	uint8_t ks_ar;
	uint8_t am_dr;
	uint8_t sr;
	uint8_t sl_rr;
	uint8_t ssg_eg;
};

struct opn_voice {
	uint8_t fb_connect;
	int vgm_ofs;
	uint8_t chan_used_mask;
	struct opn_voice_operator operators[4];
};

struct opn_analyzer {
	struct chip_analyzer chip_analyzer;

	int clock;
	uint8_t regs[512];

	struct opn_voice *voices;
	int num_voices;
	int ym_dac;
	int num_channels;
};

struct opn_analyzer *opn_analyzer_new(int clock, int num_channels);
void opn_analyzer_init(struct opn_analyzer *analyzer, int clock, int num_channels);
void opn_analyzer_dump_voices(struct opn_analyzer *analyzer);
